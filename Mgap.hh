#ifndef MGAP_H
#define MGAP_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"
#include "GpuTopology.hh"
#include "TopoUtils.hh"
#include "MgapPolicies.hh"

Nodes busyNodes;
JobVec jobList;
JobVec jobQueue;     // Ready to be scheduled.
JobVec jobScheduled; // Scheduled/Running jobs.
JobVec jobFinished;  // Finished jobs.

extern SmallGraph currTopo;
extern SmallGraph hwTopo;
extern BwMap bwmap;

bool isFinished(JobItem job, uint32_t cycles)
{
  uint32_t currTime;
  currTime = job.startTime + job.srvcTime;
  job.endTime = cycles;
  return (cycles == currTime);
}

template <typename Vec, typename T>
void erase(Vec& vec, T item)
{
  vec.erase(std::remove_if(vec.begin(), vec.end(),
                           [item](auto &elem) { return elem.getId() == item.getId(); }),
            vec.end());
}

template <typename Vec, typename T>
void moveItem(Vec& destVec, Vec& sourceVec, T item)
{
  destVec.emplace_back(item);
  erase(sourceVec, item);
}

void findPatterns(SmallGraph topo, std::vector<SmallGraph> appTopo)
{
  using namespace Peregrine;
  size_t nthreads = 1;
  std::vector<uint32_t> testingVec;
  const auto callback = [](auto &&handle, auto &&match)
  {
    handle.map(match.pattern, 1);
    utils::store_pattern(match.mapping);
  };
  auto results = match<Pattern, uint64_t, ON_THE_FLY, UNSTOPPABLE>(topo, appTopo, nthreads, callback);
}

Allocation choosePattern(PatternVec patterns, JobItem job)
{
  // Policy-1: Choose the one with highest LAST score.
  if (!patterns.empty())
  {
    for (auto& busynode: busyNodes)
    {
      patterns.erase(std::remove_if(patterns.begin(), patterns.end(),
                               [busynode](auto &pattern) { return std::find(pattern.begin(), pattern.end(), busynode) != pattern.end(); }),
                     patterns.end());
    }
    return policyMap[POLICYNAME](patterns, job);
  }
  else
  {
    return {};
  }
}

uint32_t simulate(std::string jobsFilename, GpuSystem gpuSys)
{
  bwmap = gpuSys.bwmap;
  currTopo = gpuSys.topology;
  hwTopo = gpuSys.topology;

  readJobFile(jobsFilename);

  std::cout << "Using Policy: " << POLICYNAME << std::endl;

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
          // addNodes(jobIt->schedGPUs);
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
      auto alloc = choosePattern(utils::foundPatterns, job);
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
          busyNodes.push_back(node);
        }
        moveItem(jobScheduled, jobQueue, job);
        break;
      }
    }
    cycles++;
  }
  logging(jobFinished, 2);
  return cycles;
}

// uint32_t realRun()
// {
//   // Add code to take an arbitrary function to schedule via MGAP.
// }

#endif