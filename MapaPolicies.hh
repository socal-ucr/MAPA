#ifndef MAPA_POLICIES_HH
#define MAPA_POLICIES_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"
#include "GpuTopology.hh"
#include "TopoUtils.hh"

Allocation preserveScoreMax(PatternVec& patterns, JobItem job)
{
  Allocation alloc = {};
  for (auto &pattern : patterns)
  {
    // logging(pattern);
    auto tempAlloc = getAllocationForPattern(pattern, job.topology);
    if (alloc.preserveScore < tempAlloc.preserveScore)
    {
      alloc = tempAlloc;
    }
    else if (alloc.preserveScore == tempAlloc.preserveScore)
    {
      if (alloc.lastScore < tempAlloc.lastScore)
      {
        alloc = tempAlloc;
      }
    }
  }
  return alloc;
}

Allocation predictedBWgreedyMax(PatternVec &patterns, JobItem job)
{
  Allocation alloc = {};
  for (auto &pattern : patterns)
  {
    // logging(pattern);
    auto tempAlloc = getAllocationForPattern(pattern, job.topology);
    if (alloc.getPredictedBW() < tempAlloc.getPredictedBW())
    {
      alloc = tempAlloc;
    }
  }
  return alloc;
}

Allocation LASTgreedyMax(PatternVec& patterns, JobItem job)
{
  Allocation alloc = {};
  for (auto &pattern : patterns)
  {
    // logging(pattern);
    auto tempAlloc = getAllocationForPattern(pattern, job.topology);
    if (alloc.lastScore < tempAlloc.lastScore)
    {
      alloc = tempAlloc;
    }
  }
  return alloc;
}

Allocation LASTgreedy(PatternVec& patterns, JobItem job)
{
  Allocation alloc = {};
  logging("Iterating through Patterns in LASTgreedy policy");

  if ((job.numGpus == 1) && patterns.size())
  {
    alloc = getAllocationForPattern(patterns[0], job.topology);
  }
  else
  {
    alloc = LASTgreedyMax(patterns, job);
  }

  return alloc;
}

Allocation LASTpreserve(PatternVec &patterns, JobItem job)
{
  Allocation alloc = {};

  logging("Iterating through Patterns in LASTpreserve policy");

  if ((job.bwSensitive) && (job.numGpus > 1))
  {
    alloc = predictedBWgreedyMax(patterns, job);
  }
  else
  {
    alloc = preserveScoreMax(patterns, job);
  }

  return alloc;
}

Allocation LASTminScore(PatternVec &patterns, JobItem job)
{
  Allocation alloc = {};

  logging("Iterating through Patterns in LASTpreserve policy");

  if ((job.bwSensitive) && (job.numGpus > 1))
  {
    alloc = predictedBWgreedyMax(patterns, job);
    if ((alloc.normLastScore != 1) && (job.numGpus > 1))
    {
      return Allocation{};
    }
  }
  else
  {
    alloc = preserveScoreMax(patterns, job);
  }

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
    alloc.lastScore = getLastScoreWithRoute(alloc.pattern, job.topology);
    updateNormLastScore(alloc);
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
  alloc.preserveScore = 0;

  logging("Iterating through Patterns in policy");

  if (job.bwSensitive)
  {
    alloc = LASTgreedyRoute(patterns, job);
  }
  else
  {
    for (auto &pattern : patterns)
    {
      logging(pattern);
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
    if (alloc.pattern.size())
    {
      alloc.lastScore = getLastScoreWithRoute(alloc.pattern, job.topology);
      updateNormLastScore(alloc);
      alloc.edges = getEdges(alloc.pattern, job.topology);
    }
  }
  logging("Printing selected pattern\n");
  logging(alloc.pattern);
  return alloc;
}

Allocation baselineV1(PatternVec& patterns, JobItem job)
{
  // Pass any available alloc based on smallest/available ID.
  Allocation alloc = {};
  alloc.lastScore = 0;

  if ((job.numGpus == 1) && patterns.size())
  {
    alloc = getAllocationForPattern(patterns[0], job.topology);
  }
  else
  {
    if (patterns.size())
    {
      auto pattern = patterns[0];
      logging(pattern);
      do
      {
        auto tempAlloc = getAllocationForPattern(pattern, job.topology);
        if (alloc.lastScore < tempAlloc.lastScore)
        {
          alloc = tempAlloc;
        }
      } while (next_permutation(pattern.begin(), pattern.end()));
    }
  }

  return alloc;
}

Allocation baselineV2(PatternVec& patterns, JobItem job)
{
  // Pass an alloc in the same PCIe root complex, if none fallback to baselineV1.
  Allocation alloc = {};
  if ((job.numGpus == 1) && patterns.size())
  {
    alloc = getAllocationForPattern(patterns[0], job.topology);
  }
  else
  {
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
                alloc = getAllocationForPattern(pattern, job.topology);
                return alloc;
              }
            }
          }
        }
      }
      return baselineV1(patterns, job);
    }
  }
  return alloc;
}

std::map<std::string, std::function<Allocation(PatternVec &, JobItem)>> policyMap =
    {{"LASTgreedy", [](PatternVec &patterns, JobItem job) { return LASTgreedy(patterns, job); }},
     {"LASTminScore", [](PatternVec &patterns, JobItem job) { return LASTminScore(patterns, job); }},
     {"LASTgreedyRoute", [](PatternVec &patterns, JobItem job) { return LASTgreedyRoute(patterns, job); }},
     {"LASTpreserve", [](PatternVec &patterns, JobItem job) { return LASTpreserve(patterns, job); }},
     {"LASTpreserveRoute", [](PatternVec &patterns, JobItem job) { return LASTpreserveRoute(patterns, job); }},
     {"baselineV1", [](PatternVec &patterns, JobItem job) { return baselineV1(patterns, job); }},
     {"baselineV2", [](PatternVec &patterns, JobItem job) { return baselineV2(patterns, job); }}};

#endif
