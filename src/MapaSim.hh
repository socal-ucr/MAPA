#ifndef MAPASIM_H
#define MAPASIM_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include "Mapa.hh"

Nodes busyNodes;
JobVec jobList;
JobVec waitingJobs;     // Ready to be scheduled.
JobVec runningJobs; // Scheduled/Running jobs.
// TODO(Kiran): Purge jobFinsihed Data structure
JobVec jobFinished;  // Finished jobs.

std::string logFilename;

long int cycles = 0;
int totalGpus;

extern SmallGraph hwTopo;
extern BwMap bwmap;
extern RouteBWmap routeBWmap;
extern std::map<uint32_t, uint32_t> idealLastScore;

bool isFinished(JobItem job, uint32_t cycles)
{
  uint32_t currTime;
  currTime = job.startTime + job.srvcTime;
  job.endTime = cycles;
  return (cycles == currTime);
}

void checkCompletedJobs()
{
  auto jobIt = runningJobs.begin();
  while (jobIt != runningJobs.end())
  {
    if (isFinished(*jobIt, cycles))
    {
      logging("Finished Job " + std::to_string(jobIt->getId()) + " at " + std::to_string(cycles));
      jobIt->endTime = cycles;
      jobIt->execTime = cycles - jobIt->startTime;
      for (auto &node : jobIt->schedGPUs)
      {
        busyNodes.erase(std::remove_if(busyNodes.begin(), busyNodes.end(),
                                       [node](auto &elem) { return node == elem; }),
                        busyNodes.end());
      }
      logresult(*jobIt, logFilename);
      moveItem(jobFinished, runningJobs, *jobIt);
      jobIt = runningJobs.begin();
    }
    else
    {
      jobIt++;
    }
  }

  return;
}

void populatewaitingJobs()
{
  // for (auto it = jobList.begin(); it != jobList.end();)
  // {
  //   if (it->arvlTime <= cycles)
  //   {
  //     waitingJobs.emplace_back(*it);
  //     erase(jobList, *it);
  //     it = jobList.begin();
  //   }
  //   else if (it->arvlTime > cycles)
  //   {
  //     break;
  //   }
  //   else
  //   {
  //     ++it;
  //   }
  // }
  for (auto job : jobList)
  {
    job.arvlTime = 0; // Remove this later on.
    waitingJobs.emplace_back(job);
  }
  jobList.clear();

  return;
}

void scheduleReadyJobs(std::string mapaPolicy)
{
  for (auto &job : waitingJobs)
  {
    // logging("Available GPUs " + std::to_string(totalGpus - busyNodes.size()));
    // logging("Required GPUs " + std::to_string(job.numGpus));

    // TODO(Kiran): arvlTime set will be removed in the future. This is done to test the queueTime with stall policy.
    if (job.arvlTime == 0)
    {
      job.arvlTime = cycles;
    }

    if (job.numGpus > (totalGpus - busyNodes.size()))
    {
      logging("Insufficient GPUs");
      break;
    }

    logging("Finding Allocation for Job " + std::to_string(job.getId()));
    findPatterns(hwTopo, job.numGpus, job.pattern);
    // utils::print_patterns();
    auto matchingPatterns = filterPatterns(utils::foundPatterns, busyNodes);
    auto alloc = choosePattern(matchingPatterns, job, mapaPolicy);

    if (alloc.pattern.empty())
    {
      logging("Allocation not found");
      break; // Ensure the first job in the queue blocks until scheduled.
    }
    else
    {
      logging("Scheduled Job " + std::to_string(job.getId()) + "at " + std::to_string(cycles));
      job.startTime = cycles;
      job.queueTime = job.arvlTime - job.startTime;
      logging("Allocation found");
      logging(alloc.pattern);
      job.alloc = alloc;
      for (auto &node : alloc.pattern)
      {
        job.schedGPUs.push_back(node);
        busyNodes.push_back(node);
      }
      moveItem(runningJobs, waitingJobs, job);
      // break; // Removing break will try to schedule the next available job.
    }
  }

  return;
}

void run(std::string jobsFilename, GpuSystem gpuSys, std::string mapaPolicy)
{
  totalGpus = gpuSys.numGpus;
  bwmap = gpuSys.bwmap;
  routeBWmap = gpuSys.routeBWmap;
  hwTopo = gpuSys.topology;
  idealLastScore = gpuSys.idealLastScore;

  readJobFile(jobsFilename);

  logFilename = jobsFilename + gpuSys.name + mapaPolicy + "Log.csv";

  createLogFile(logFilename);

  std::cout << "Starting simulation" << std::endl << std::endl;
  std::cout << "Jobfile: " << jobsFilename << std::endl;
  std::cout << "Using Policy: " << mapaPolicy << std::endl << std::endl;

  while (!jobList.empty() || !waitingJobs.empty() || !runningJobs.empty())
  {
    logging("Cycle = " + std::to_string(cycles));
    if (!runningJobs.empty())
    {
      checkCompletedJobs();
    }
    
    if (jobList.size())
    {
      populatewaitingJobs();
    }

    if (waitingJobs.size())
    {
      scheduleReadyJobs(mapaPolicy);
    }

    cycles++;
  }

  uint32_t avgLS = 0;
  double avgFS = 0;

  for (auto& job : jobFinished)
  {
    avgLS += job.alloc.lastScore;
    avgFS += job.alloc.normLastScore;
  }

  avgLS /= jobFinished.size();
  avgFS /= jobFinished.size();

  std::cout << "Average Last Score " << avgLS << std::endl;
  std::cout << "Average Frag Score " << avgFS << std::endl;
  std::cout << "Logging results to " << logFilename << std::endl;

  return;
}

#endif
