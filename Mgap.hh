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

void filterPatterns(PatternVec& patterns, const Nodes& busyNodes)
{
  for (auto &busynode : busyNodes)
  {
    patterns.erase(std::remove_if(patterns.begin(), patterns.end(),
                                  [busynode](auto &pattern) { return std::find(pattern.begin(), pattern.end(), busynode) != pattern.end(); }),
                   patterns.end());
  }
}

Allocation choosePattern(PatternVec& patterns, JobItem job, std::string mgapPolicy)
{
  if (!patterns.empty())
  {
    return policyMap[mgapPolicy](patterns, job);
  }
  else
  {
    return {};
  }
}

// uint32_t realRun()
// {
//   // Add code to take an arbitrary function to schedule via MGAP.
// }

#endif