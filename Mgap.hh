#ifndef MGAP_H
#define MGAP_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"
#include "GpuTopology.hh"
#include "Utils.hh"

// Nodes busyNodes;
JobVec jobList;
JobVec jobQueue;     // Ready to be scheduled.
JobVec jobScheduled; // Scheduled jobs.
JobVec jobFinished;  // Finished jobs.

SmallGraph currTopo;
SmallGraph hwTopo;
BwMap bwmap;

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

Allocation largestLastScore(PatternVec patterns, std::string topology)
{
  Allocation alloc;

  for (auto &pattern : patterns)
  {
    uint32_t currlastScore = getLastScore(pattern, topology);
    if (alloc.lastScore < currlastScore)
    {
      alloc.pattern = pattern;
      alloc.lastScore = currlastScore;
    }
  }
  return alloc;
}

void findPatterns(SmallGraph hwTopo, std::vector<SmallGraph> appTopo)
{
  using namespace Peregrine;
  size_t nthreads = 1;
  std::vector<uint32_t> testingVec;
  const auto callback = [](auto &&handle, auto &&match)
  {
    handle.map(match.pattern, 1);
    utils::store_pattern(match.mapping);
  };
  auto results = match<Pattern, uint64_t, ON_THE_FLY, UNSTOPPABLE>(hwTopo, appTopo, nthreads, callback);
}

Allocation choosePattern(PatternVec patterns, std::string topology)
{
  // Policy-1: Choose the one with highest LAST score.
  if (!patterns.empty())
  {
    return largestLastScore(patterns, topology);
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

  uint32_t cycles = 0;

  while (!jobList.empty() || !jobQueue.empty() || !jobScheduled.empty())
  {
    if (!jobScheduled.empty())
    {
      auto jobIt = jobScheduled.begin();
      while (jobIt != jobScheduled.end())
      {
        if (isFinished(*jobIt, cycles))
        {
          moveItem(jobFinished, jobScheduled, *jobIt);
          addNodes(jobIt->schedGPUs);
          jobIt = jobScheduled.begin();
          // for (auto &node : job.schedGPUs)
          // {
          //   erase(busyNodes, node);
          // }
        }
        else 
        {
          jobIt++;
        }
      }
    }
    for (auto i = 0; jobList[i].arvlTime == cycles; ++i)
    {
      jobQueue.emplace_back(jobList[i]);
    }
    for (auto& job : jobQueue)
    {
      findPatterns(currTopo, job.pattern);
      Allocation alloc = choosePattern(utils::foundPatterns, job.topology);
      if (!alloc.pattern.empty())
      {
        removeNodes(alloc.pattern);
        job.startTime = cycles;
        std::cout << "Allocation found" << std::endl;
        utils::print_vector(alloc.pattern);
        // add busy nodes.
        for (auto& node : alloc.pattern)
        {
          job.schedGPUs.push_back(node);
        }
        job.startTime = cycles;
        moveItem(jobScheduled, jobQueue, job);
        break;
      }
    }
    cycles++;
  }
  return cycles;
}

// uint32_t realRun()
// {
//   // Add code to take an arbitrary function to schedule via MGAP.
// }

#endif