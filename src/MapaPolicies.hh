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

extern uint32_t numGpusPerNode;
extern SmallGraph hwTopo;

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

Allocation greedy(PatternVec& patterns, JobItem job)
{
  Allocation alloc = {};
  logging("Iterating through Patterns in greedy policy");

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

Allocation preserve(PatternVec &patterns, JobItem job)
{
  Allocation alloc = {};

  logging("Iterating through Patterns in preserve policy");

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

Allocation minScore(PatternVec &patterns, JobItem job)
{
  Allocation alloc = {};

  logging("Iterating through Patterns in preserve policy");

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

Allocation greedyRoute(PatternVec &patterns, JobItem job)
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

Allocation preserveRoute(PatternVec& patterns, JobItem job)
{
  Allocation alloc;
  alloc.lastScore = 0;
  alloc.preserveScore = 0;

  logging("Iterating through Patterns in policy");

  if (job.bwSensitive)
  {
    alloc = greedyRoute(patterns, job);
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

Allocation baseline(PatternVec& patterns, JobItem job)
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

bool patternSpreadOut(uint32_t initial, uint32_t test)
{
  uint32_t numVertices = hwTopo.num_vertices();
  uint32_t cmp = 0;
  for (uint32_t start=1; start < numVertices; start += numGpusPerNode)
  {
    if ((initial >= start) && (initial < start + numGpusPerNode))
    {
      cmp = start;
      break;
    }
  }
  
  return ((test < cmp) || (test >= cmp + numGpusPerNode));
}

Allocation topoAware(PatternVec& patterns, JobItem job)
{
  // Pass an alloc in the same PCIe root complex, if none fallback to baseline.
  Allocation alloc = {};
  if ((job.numGpus == 1) && patterns.size())
  {
    alloc = getAllocationForPattern(patterns[0], job.topology);
  }
  else
  {
    if (patterns.size())
    {
      if (job.numGpus < numGpusPerNode)
      {
        for (auto &pattern : patterns)
        {
          logging(pattern);
          for (auto node : pattern)
          {
            if (patternSpreadOut(pattern[0], node))
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
      return baseline(patterns, job);
    }
  }
  return alloc;
}

std::map<std::string, std::function<Allocation(PatternVec &, JobItem)>> policyMap =
    {{"greedy", [](PatternVec &patterns, JobItem job) { return greedy(patterns, job); }},
     {"minScore", [](PatternVec &patterns, JobItem job) { return minScore(patterns, job); }},
     {"greedyRoute", [](PatternVec &patterns, JobItem job) { return greedyRoute(patterns, job); }},
     {"preserve", [](PatternVec &patterns, JobItem job) { return preserve(patterns, job); }},
     {"preserveRoute", [](PatternVec &patterns, JobItem job) { return preserveRoute(patterns, job); }},
     {"baseline", [](PatternVec &patterns, JobItem job) { return baseline(patterns, job); }},
     {"topoAware", [](PatternVec &patterns, JobItem job) { return topoAware(patterns, job); }}};

#endif
