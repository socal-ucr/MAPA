#ifndef MGAPREAL_H
#define MGAPREAL_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include "Mgap.hh"

Nodes busyNodes;
JobVec jobList;
JobVec jobQueue;     // Ready to be scheduled.
JobVec jobScheduled; // Scheduled/Running jobs.
JobVec jobFinished;  // Finished jobs.

extern SmallGraph currTopo;
extern SmallGraph hwTopo;
extern BwMap bwmap;

void forkProcess()
{
  return;
}

uint32_t realRun(std::string jobsFilename, GpuSystem gpuSys, std::string mgapPolicy)
{
  bwmap = gpuSys.bwmap;
  currTopo = gpuSys.topology;
  hwTopo = gpuSys.topology;

  readJobFile(jobsFilename);

  std::cout << "Starting Run" << std::endl << std::endl;
  std::cout << "Jobfile: " << jobsFilename << std::endl;
  std::cout << "Using Policy: " << mgapPolicy << std::endl << std::endl;

  uint32_t cycles = 0;

  while (!jobList.empty() || !jobQueue.empty() || !jobScheduled.empty())
  {
    logging("Cycle = " + std::to_string(cycles), 1);
    if (!jobScheduled.empty())
    {
      auto jobIt = jobScheduled.begin();
      while (jobIt != jobScheduled.end())
      {
        if (isFinished(*jobIt, cycles))
        {
          logging("Finished Job " + std::to_string(jobIt->getId()) + " at " + std::to_string(cycles), 1);
          jobIt->endTime = cycles;
          for (auto &node : jobIt->schedGPUs)
          {
            busyNodes.erase(std::remove_if(busyNodes.begin(), busyNodes.end(),
                                           [node](auto &elem) { return node == elem; }),
                            busyNodes.end());
          }
          moveItem(jobFinished, jobScheduled, *jobIt);
          jobIt = jobScheduled.begin();
        }
        else 
        {
          jobIt++;
        }
      }
    }
    for (auto it = jobList.begin(); it != jobList.end();)
    {
      if (it->arvlTime <= cycles)
      {
        jobQueue.emplace_back(*it);
        erase(jobList, *it);
        it = jobList.begin();
      }
      else if (it->arvlTime > cycles)
      {
        break;
      }
      else
      {
        ++it;
      }
    }
    
    for (auto& job : jobQueue)
    {
      logging("Available GPUs " + std::to_string(gpuSys.numGpus - busyNodes.size()), 1);
      logging("Required GPUs " + std::to_string(job.numGpus), 1);
      if (job.numGpus > (gpuSys.numGpus - busyNodes.size()))
      {
        logging("Insufficient GPUs", 1);
        break;
      }
      logging("Finding Allocation for Job " + std::to_string(job.getId()), 2);
      findPatterns(currTopo, job.pattern);
      // utils::print_patterns();
      filterPatterns(utils::foundPatterns, busyNodes);
      auto alloc = choosePattern(utils::foundPatterns, job, mgapPolicy);
      utils::clear_patterns();
      if (!alloc.pattern.empty())
      {
        // Check getID() it is returning garbage.
        logging("Scheduled Job " + std::to_string(job.getId()) + "at "+ std::to_string(cycles), 1);
        // removeNodes(alloc.pattern);
        job.startTime = cycles;
        logging("Allocation found", 1);
        logging(alloc.pattern, 1);
        // add busy nodes. TODO: IMPLEMENT WITH BUSY NODES.
        for (auto& node : alloc.pattern)
        {
          job.schedGPUs.push_back(node);
          job.alloc = alloc;
          busyNodes.push_back(node);
        }
        moveItem(jobScheduled, jobQueue, job);
        break;
      }
    }
    cycles++;
  }
  uint32_t avgLS = 0;
  double avgFS = 0;
  for (auto& job : jobFinished)
  {
    avgLS += job.alloc.lastScore;
    job.alloc.fragScore = 1 - (static_cast<double>(job.alloc.lastScore) / (static_cast<double>(gpuSys.idealLastScore) * job.numGpus));
    avgFS += job.alloc.fragScore;
  }
  avgLS /= jobFinished.size();
  avgFS /= jobFinished.size();

  std::string logFilename = jobsFilename + mgapPolicy + "Log.csv";
  std::cout << "Average Last Score " << avgLS << std::endl;
  std::cout << "Average Frag Score " << avgFS << std::endl;
  std::cout << "Logging results to " << logFilename << std::endl;
  logresults(jobFinished, 2, logFilename);
  return cycles;
}

#endif