#ifndef MGAPREAL_H
#define MGAPREAL_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <sys/wait.h>

#include "Mgap.hh"

struct RunningJob
{
  JobItem job;
  int pid;
  int status;

  bool operator==(const RunningJob &a) const
  {
    return job.getId() == a.job.getId();
  }
};

Nodes busyNodes;
JobVec jobList;
JobVec jobQueue;     // Ready to be scheduled.
std::vector<RunningJob> jobScheduled; // Scheduled/Running jobs.
JobVec jobFinished;  // Finished jobs.
int numGpus;

extern SmallGraph currTopo;
extern SmallGraph hwTopo;
extern BwMap bwmap;

void forkProcess(RunningJob& rjob)
{
  // int pid, status;
  // first we fork the process
  rjob.pid = fork();
  if (rjob.pid)
  {
    return;
  }
  else
  {
    /* pid == 0: this is the child process. now let's load the
       program into this process and run it */
    char* cmd = const_cast<char*>(rjob.job.taskToRun.c_str());
    std::string nodes;
    for (auto node : rjob.job.schedGPUs)
    {
      nodes += std::to_string(node) + ",";
    }
    char *argv[3] = {cmd, const_cast<char *>(nodes.c_str()), NULL};
    // const char dir[] = "$HOME/workspace/caffe-scripts";

    // load it. there are more exec__ functions, try 'man 3 exec'
    // execl takes the arguments as parameters. execv takes them as an array
    // this is execl though, so:
    //      exec         argv[0]  argv[1] end
    execvp(cmd, argv);
  }
}

// __pid_t isFinished(RunningJob* rjob)
// {
//   return waitpid(rjob->pid, &(rjob->status), WNOHANG);
// }

long int getTimeNow()
{
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  auto epoch = now_ms.time_since_epoch();
  auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
  return value.count();
}

void checkCompletedJobs()
{
  auto rjobIt = jobScheduled.begin();
  while (rjobIt != jobScheduled.end())
  {
    if (waitpid(rjobIt->pid, &(rjobIt->status), WNOHANG))
    {
      (rjobIt->job).endTime = getTimeNow();
      logging("Finished Job " + std::to_string((rjobIt->job).getId()) + " at " + std::to_string((rjobIt->job).endTime - (rjobIt->job).startTime), 1);
      for (auto &node : (rjobIt->job).schedGPUs)
      {
        busyNodes.erase(std::remove_if(busyNodes.begin(), busyNodes.end(),
                                       [node](auto &elem) { return node == elem; }),
                        busyNodes.end());
      }
      jobFinished.emplace_back(rjobIt->job);
      erase(jobScheduled, *rjobIt);
      rjobIt = jobScheduled.begin();
    }
    else
    {
      rjobIt++;
    }
  }

  return;
}

void populateJobQueue()
{
  // TODO: We are not considering arvlTime in this implementation.
  for (auto job : jobList)
  {
    job.arvlTime = getTimeNow();
    jobQueue.emplace_back(job);
  }
  jobList.clear();

  return;
}

void scheduleReadyJobs(std::string mgapPolicy)
{
  for (auto &job : jobQueue)
  {
    logging("Available GPUs " + std::to_string(numGpus - busyNodes.size()), 1);
    logging("Required GPUs " + std::to_string(job.numGpus), 1);
    if (job.numGpus > (numGpus - busyNodes.size()))
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
      job.startTime = getTimeNow();
      logging("Scheduled Job " + std::to_string(job.getId()) + "at " + std::to_string(job.startTime - job.arvlTime), 1);
      logging("Allocation found", 1);
      logging(alloc.pattern, 1);
      job.alloc = alloc;
      for (auto &node : alloc.pattern)
      {
        job.schedGPUs.push_back(node);
        busyNodes.push_back(node);
      }
      RunningJob rjob{job, 0, 0};
      forkProcess(rjob); // FORK JOB
      jobScheduled.emplace_back(rjob);
      erase(jobQueue, job);
      break;
    }
  }

  return;
}

uint32_t realRun(std::string jobsFilename, GpuSystem gpuSys, std::string mgapPolicy)
{
  bwmap = gpuSys.bwmap;
  currTopo = gpuSys.topology;
  hwTopo = gpuSys.topology;
  numGpus = gpuSys.numGpus;

  readJobFile(jobsFilename);

  std::cout << "Starting Run" << std::endl << std::endl;
  std::cout << "Jobfile: " << jobsFilename << std::endl;
  std::cout << "Using Policy: " << mgapPolicy << std::endl << std::endl;

  auto startTime = getTimeNow();

  while (!jobList.empty() || !jobQueue.empty() || !jobScheduled.empty())
  {
    if (!jobScheduled.empty())
    {
      checkCompletedJobs();
    }

    if (jobList.size())
    {
      populateJobQueue();
    }

    if (jobQueue.size())
    {
      scheduleReadyJobs(mgapPolicy);
    }
  }

  auto endTime = getTimeNow();

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

  return (endTime - startTime); // Return final execTime.
}

#endif