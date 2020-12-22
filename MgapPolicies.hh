#ifndef MGAP_POLICIES_HH
#define MGAP_POLICIES_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"
#include "GpuTopology.hh"
#include "TopoUtils.hh"

constexpr char POLICYNAME[] = "bwSensitiveLastScore";

Allocation largestLastScore(PatternVec patterns, JobItem job)
{
  Allocation alloc;

  logging("Iterating through Patterns in policy", 2);

  for (auto &pattern : patterns)
  {
    logging(pattern, 1);
    uint32_t currlastScore = getLastScore(pattern, job.topology);
    logging("lastScore = " + std::to_string(currlastScore), 1);
    if (alloc.lastScore < currlastScore)
    {
      alloc.pattern = pattern;
      alloc.lastScore = currlastScore;
    }
  }
  if (alloc.pattern.size())
  {
    alloc.edges = getEdges(alloc.pattern, job.topology);
  }
  logging("Printing selected pattern\n", 1);
  logging(alloc.pattern, 1);
  return alloc;
}

Allocation bwSensitiveLastScore(PatternVec patterns, JobItem job)
{
  Allocation alloc;

  logging("Iterating through Patterns in policy", 2);
  for (auto &pattern : patterns)
  {
    logging(pattern, 1);
    uint32_t currlastScore = getLastScore(pattern, job.topology);
    logging("lastScore = " + std::to_string(currlastScore), 1);

    if (job.bwSensitive)
    {
      if (alloc.lastScore < currlastScore)
      {
        alloc.pattern = pattern;
        alloc.lastScore = currlastScore;
      }
    }
    else
    {
      if (alloc.lastScore == 0)
      {
        alloc.pattern = pattern;
        alloc.lastScore = currlastScore;
      }
      else if (alloc.lastScore > currlastScore)
      {
        alloc.pattern = pattern;
        alloc.lastScore = currlastScore;
      }
    }
  }
  if (alloc.pattern.size())
  {
    alloc.edges = getEdges(alloc.pattern, job.topology);
  }
  logging("Printing selected pattern\n", 1);
  logging(alloc.pattern, 1);
  return alloc;
}

std::map<std::string, std::function<Allocation(PatternVec, JobItem)>> policyMap =
    {{"largestLastScore", [](PatternVec patterns, JobItem job) { return largestLastScore(patterns, job); }},
     {"bwSensitiveLastScore", [](PatternVec patterns, JobItem job){ return bwSensitiveLastScore(patterns,  job); }}
    };

#endif