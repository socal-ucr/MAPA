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

Allocation LASTgreedy(PatternVec& patterns, JobItem job)
{
  Allocation alloc;
  alloc.lastScore = 0;

  logging("Iterating through Patterns in policy");

  for (auto &pattern : patterns)
  {
    logging(pattern);
    uint32_t currlastScore = getLastScore(pattern, job.topology);
    logging("lastScore = " + std::to_string(currlastScore));
    if (alloc.lastScore < currlastScore)
    {
      alloc.pattern = pattern;
      alloc.lastScore = currlastScore;
    }
  }
  if (alloc.pattern.size())
  {
    updateFragScore(alloc);
    alloc.edges = getEdges(alloc.pattern, job.topology);
  }
  logging("Selected pattern\n");
  logging(alloc.pattern);
  return alloc;
}

Allocation LASTgreedyRoute(PatternVec &patterns, JobItem job)
{
  Allocation alloc;
  alloc.lastScore = 0;

  logging("Iterating through Patterns in policy");

  for (auto &pattern : patterns)
  {
    logging(pattern);
    uint32_t currlastScore = getLastScoreWithRoute(pattern, job.topology);
    logging("lastScore = " + std::to_string(currlastScore));
    if (alloc.lastScore < currlastScore)
    {
      alloc.pattern = pattern;
      alloc.lastScore = currlastScore;
    }
  }
  if (alloc.pattern.size())
  {
    updateFragScore(alloc);
    alloc.edges = getEdges(alloc.pattern, job.topology);
  }
  logging("Selected pattern\n");
  logging(alloc.pattern);
  return alloc;
}

Allocation LASTpreserveRoute(PatternVec& patterns, JobItem job)
{
  Allocation alloc;
  alloc.lastScore = 0;

  logging("Iterating through Patterns in policy");
  for (auto &pattern : patterns)
  {
    logging(pattern);
    if (job.bwSensitive)
    {
      uint32_t currlastScore = getLastScoreWithRoute(pattern, job.topology);
      logging("lastScore = " + std::to_string(currlastScore));
      if (alloc.lastScore < currlastScore)
      {
        alloc.pattern = pattern;
        alloc.lastScore = currlastScore;
      }
    }
    else
    {
      uint32_t currpScore = getPreservationScore(pattern);
      logging("preserveScore = " + std::to_string(currpScore));
      if (alloc.preserveScore < currpScore)
      {
        alloc.pattern = pattern;
        alloc.preserveScore = currpScore;
        alloc.lastScore = getLastScoreWithRoute(pattern, job.topology);
      }
    }
  }
  if (alloc.pattern.size())
  {
    updateFragScore(alloc);
    alloc.edges = getEdges(alloc.pattern, job.topology);
  }
  logging("Printing selected pattern\n");
  logging(alloc.pattern);
  return alloc;
}

Allocation LASTpreserve(PatternVec& patterns, JobItem job)
{
  Allocation alloc;
  alloc.lastScore = 0;

  logging("Iterating through Patterns in policy");
  for (auto &pattern : patterns)
  {
    logging(pattern);
    if (job.bwSensitive)
    {
      uint32_t currlastScore = getLastScore(pattern, job.topology);
      logging("lastScore = " + std::to_string(currlastScore));
      if (alloc.lastScore < currlastScore)
      {
        alloc.pattern = pattern;
        alloc.lastScore = currlastScore;
      }
    }
    else
    {
      uint32_t currpScore = getPreservationScore(pattern);
      logging("preserveScore = " + std::to_string(currpScore));
      if (alloc.preserveScore < currpScore)
      {
        alloc.pattern = pattern;
        alloc.preserveScore = currpScore;
        alloc.lastScore = getLastScore(pattern, job.topology);
      }
      else if (alloc.preserveScore == currpScore)
      {
        auto currLastScore = getLastScore(pattern, job.topology);
        if (alloc.lastScore < currLastScore)
        {
          alloc.pattern = pattern;
          alloc.preserveScore = currpScore;
          alloc.lastScore = currLastScore;
        }
      }
    }
  }
  if (alloc.pattern.size())
  {
    updateFragScore(alloc);
    alloc.edges = getEdges(alloc.pattern, job.topology);
  }
  logging("Printing selected pattern\n");
  logging(alloc.pattern);
  return alloc;
}

Allocation LASTbw(PatternVec& patterns, JobItem job)
{
  Allocation alloc;

  logging("Iterating through Patterns in policy");
  for (auto &pattern : patterns)
  {
    logging(pattern);
    uint32_t currlastScore = getLastScore(pattern, job.topology);
    logging("lastScore = " + std::to_string(currlastScore));

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
    updateFragScore(alloc);
    alloc.edges = getEdges(alloc.pattern, job.topology);
  }
  logging("Printing selected pattern\n");
  logging(alloc.pattern);
  return alloc;
}

Allocation baselineV1(PatternVec& patterns, JobItem job)
{
  // Pass any available alloc based on smallest/available ID.
  Allocation alloc;

  if (patterns.size())
  {
    auto pattern = patterns[0];
    logging(pattern);
    alloc.pattern = pattern;
    alloc.lastScore = getLastScore(pattern, job.topology);
    updateFragScore(alloc);
    alloc.edges = getEdges(alloc.pattern, job.topology);
    logging("Printing selected pattern\n");
    logging(alloc.pattern);
  }

  return alloc;
}

Allocation baselineV2(PatternVec& patterns, JobItem job)
{
  // Pass an alloc in the same PCIe root complex, if none fallback to baselineV1.
  if (patterns.size())
  {
    if (job.numGpus < 5)
    {
      for (auto &pattern : patterns)
      {
        logging(pattern);
        for (auto node : pattern)
        {
          if (((pattern[0] < 5) && (node > 4)) || ((pattern[0] > 4) && (node < 5)))
          {
            break;
          }
          else
          {
            if (node != pattern.back())
            {
              continue;
            }
            else
            {
              Allocation alloc;
              alloc.pattern = pattern;
              alloc.lastScore = getLastScore(pattern, job.topology);
              updateFragScore(alloc);
              alloc.edges = getEdges(alloc.pattern, job.topology);
              logging("Printing selected pattern\n");
              logging(alloc.pattern);
              return alloc;
            }
          }
        }
      }
    }
    return baselineV1(patterns, job);
  }
  return Allocation {};
}

std::map<std::string, std::function<Allocation(PatternVec &, JobItem)>> policyMap =
    {{"LASTgreedy", [](PatternVec &patterns, JobItem job) { return LASTgreedy(patterns, job); }},
     {"LASTgreedyRoute", [](PatternVec &patterns, JobItem job) { return LASTgreedyRoute(patterns, job); }},
     {"LASTbw", [](PatternVec &patterns, JobItem job) { return LASTbw(patterns, job); }},
     {"LASTpreserve", [](PatternVec &patterns, JobItem job) { return LASTpreserve(patterns, job); }},
     {"LASTpreserveRoute", [](PatternVec &patterns, JobItem job) { return LASTpreserveRoute(patterns, job); }},
     {"baselineV1", [](PatternVec &patterns, JobItem job) { return baselineV1(patterns, job); }},
     {"baselineV2", [](PatternVec &patterns, JobItem job) { return baselineV2(patterns, job); }}};

#endif
