#ifndef MAPAREAL_H
#define MAPAREAL_H

#include <iostream>
#include <fstream>
#include <vector>
#include <errno.h>
#include <list>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <sys/wait.h>

#include "Mapa.hh"

Nodes busyNodes;
JobVec jobList;
JobVec waitingJobs;  // Ready to be scheduled.
JobVec runningJobs;  // Scheduled/Running jobs.
JobVec jobFinished;  // Finished jobs.
int totalGpus;

std::string logFilename;

extern SmallGraph hwTopo;
extern BwMap bwmap;
extern RouteBWmap routeBWmap;
extern std::map<uint32_t, uint32_t> idealLastScore;

int forkProcess(JobItem& job)
{
  // int pid, status;
  // first we fork the process
  auto pid = fork();
  // Pid > 0 meaning child Pid was successfully created.
  if (pid > 0)
  {
    std::cout << "PID after fork: " << pid << std::endl;
    return pid;
  }
  else
  {
    /* pid == 0: this is the child process. now let's load the
       program into this process and run it */
    char* cmd = const_cast<char*>(job.taskToRun.c_str());
    std::string nodes;
    for (auto nodeIt = job.schedGPUs.begin(); nodeIt != job.schedGPUs.end();)
    {
      nodes += std::to_string((*nodeIt)-1); // TODO: Check if this has to be decremented before.
      if (++nodeIt == job.schedGPUs.end())
      {
        break;
      }
      else
      {
        nodes += ",";
      }
    }
    char *argv[3] = {cmd, const_cast<char *>(nodes.c_str()), NULL};
    // const char dir[] = "$HOME/workspace/caffe-scripts";

    // load it. there are more exec__ functions, try 'man 3 exec'
    // execl takes the arguments as parameters. execv takes them as an array
    // this is execl though, so:
    //      exec         argv[0]  argv[1] end
    execvp(cmd, argv);
    std::cout << "Child process failed due to Error: " << strerror(errno) << std::endl;
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

int waitOn(int pid)
{
  int status;
  return waitpid(pid, &(status), WNOHANG);
}

void checkCompletedJobs()
{
  auto jobIt = runningJobs.begin();
  while (jobIt != runningJobs.end())
  {
    if (waitOn(jobIt->pid) > 0)
    {
      jobIt->endTime = getTimeNow();
      jobIt->execTime = jobIt->endTime - jobIt->startTime;
      std::cout << "Job PID " << jobIt->pid << std::endl;
      std::cout << "Finished Job " + std::to_string(jobIt->getId()) + " at " + std::to_string(jobIt->execTime) << std::endl;
      logging("Finished Job " + std::to_string(jobIt->getId()) + " at " + std::to_string(jobIt->execTime));
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
  // TODO: We are not considering arvlTime in this implementation.
  for (auto job : jobList)
  {
    // job.arvlTime = getTimeNow();
    job.arvlTime = 0;
    waitingJobs.emplace_back(job);
  }
  jobList.clear();

  return;
}

void scheduleReadyJobs(std::string mapaPolicy)
{
  for (auto &job : waitingJobs)
  {
    if (job.arvlTime == 0)
    {
      job.arvlTime = getTimeNow();
    }

    logging("Available GPUs " + std::to_string(totalGpus - busyNodes.size()));
    logging("Required GPUs " + std::to_string(job.numGpus));
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
      job.startTime = getTimeNow();
      job.queueTime = job.arvlTime - job.startTime;
      logging("Scheduled Job " + std::to_string(job.getId()) + "at " + std::to_string(job.startTime - job.arvlTime));
      logging("Allocation found");
      logging(alloc.pattern);
      job.alloc = alloc;
      for (auto &node : alloc.pattern)
      {
        job.schedGPUs.push_back(node);
        busyNodes.push_back(node);
      }
      // RunningJob rjob;
      job.pid = forkProcess(job); // FORK JOB
      // runningJobs.emplace_back(rjob);
      // erase(waitingJobs, job);
      moveItem(runningJobs, waitingJobs, job);
      break;
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

  std::cout << "Starting Run" << std::endl << std::endl;
  std::cout << "Jobfile: " << jobsFilename << std::endl;
  std::cout << "Using Policy: " << mapaPolicy << std::endl << std::endl;

  while (!jobList.empty() || !waitingJobs.empty() || !runningJobs.empty())
  {
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
