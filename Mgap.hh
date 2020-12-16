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

Allocation largestLastScore(PatternVec patterns, std::string topology)
{
  Allocation alloc;

  for (auto &pattern : patterns)
  {
    log("Iterating through Patterns in policy", 1);
    for (auto i : pattern)
    {
      std::cout << i << " " << std::endl;
    }
    uint32_t currlastScore = getLastScore(pattern, topology);
    log("lastScore = " + std::to_string(currlastScore), 1);
    if (alloc.lastScore < currlastScore)
    {
      alloc.pattern = pattern;
      alloc.lastScore = currlastScore;
    }
  }
  log("Printing selected pattern\n", 1);
  for (auto i : alloc.pattern)
  {
    std::cout << i << " " << std::endl;
  }
  std::cout << std::endl;
  return alloc;
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
  std::cout << "HWtopo = " << std::endl;
  for (auto &node : topo.v_list())
  {
    std::cout << node << " ";
  }
  std::cout << std::endl;
  auto results = match<Pattern, uint64_t, ON_THE_FLY, UNSTOPPABLE>(topo, appTopo, nthreads, callback);
}

Allocation choosePattern(PatternVec patterns, std::string topology)
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
    log("Cycle = " + std::to_string(cycles), 1);
    if (!jobScheduled.empty())
    {
      auto jobIt = jobScheduled.begin();
      while (jobIt != jobScheduled.end())
      {
        if (isFinished(*jobIt, cycles))
        {
          log("Finished Job " + std::to_string(jobIt->getId()) + " at " + std::to_string(cycles), 1);
          // addNodes(jobIt->schedGPUs);
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
      std::cout << "Available GPUs " << (8 - busyNodes.size()) << std::endl;
      std::cout << "Required GPUs " << job.numGpus << std::endl;
      if (job.numGpus > (8 - busyNodes.size()))
      {
        std::cout << "Insufficient GPUs" << std::endl;
        break;
      }
      log("Finding Allocation for Job " + std::to_string(job.getId()), 1);
      findPatterns(currTopo, job.pattern);
      utils::print_patterns();
      auto alloc = choosePattern(utils::foundPatterns, job.topology);
      utils::clear_patterns();
      if (!alloc.pattern.empty())
      {
        // Check getID() it is returning garbage.
        log("Scheduled Job " + std::to_string(job.getId()) + "at "+ std::to_string(cycles), 1);
        // removeNodes(alloc.pattern);
        job.startTime = cycles;
        std::cout << "Allocation found" << std::endl;
        utils::print_vector(alloc.pattern);
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
  return cycles;
}

// uint32_t realRun()
// {
//   // Add code to take an arbitrary function to schedule via MGAP.
// }

#endif