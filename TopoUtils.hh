#ifndef TOPOUTILS_HH
#define TOPOUTILS_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"
#include "Props.hh"

extern BwMap bwmap;
extern RouteBWmap routeBWmap;
extern SmallGraph hwTopo;
extern std::map<uint32_t, uint32_t> idealLastScore;
extern Nodes busyNodes;

void readJobFile(std::string fname)
{
  // Format: numGpus, topology, arrivalTime, executionTime(for simulation), bwSensitivity
  std::ifstream jobFile(fname);
  std::string line;
  std::list<std::list<std::string>> jobs;
  while (getline(jobFile, line))
  {
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;
    std::list<std::string> job;
    while ((pos = line.find(delimiter)) != std::string::npos)
    {
      token = line.substr(0, pos);
      job.emplace_back(token);
      line.erase(0, pos + delimiter.length());
    }
    job.emplace_back(line);
    jobs.emplace_back(job);
  }
  jobFile.close();

  for (auto j : jobs)
  {
    JobItem job(j, jobList.size());
    jobList.emplace_back(job);
  }
}

template <typename Vec, typename T>
void erase(Vec &vec, T item)
{
  vec.erase(std::remove_if(vec.begin(), vec.end(),
                           [item](auto &elem) { return elem.getId() == item.getId(); }),
            vec.end());
}

template <typename Vec>
void erase(Vec &vecA, Vec &vecB)
{
  for (auto elemB : vecB)
  {
    vecA.erase(std::remove_if(vecA.begin(), vecA.end(),
                            [elemB](auto &elemA) { return elemA == elemB; }),
              vecA.end());
  }
}

template <typename Vec, typename T>
void moveItem(Vec &destVec, Vec &sourceVec, T item)
{
  destVec.emplace_back(item);
  erase(sourceVec, item);
}

void findPatterns(SmallGraph topo, std::vector<SmallGraph> appTopo)
{
  using namespace Peregrine;
  size_t nthreads = 1;
  std::vector<uint32_t> testingVec;
  utils::clear_patterns();
  const auto callback = [](auto &&handle, auto &&match) {
    handle.map(match.pattern, 1);
    utils::store_pattern(match.mapping);
  };
  auto results = match<Pattern, uint64_t, ON_THE_FLY, UNSTOPPABLE>(topo, appTopo, nthreads, callback);
}

PatternVec filterPatterns(PatternVec &patterns, Nodes busyNodes = busyNodes)
{
  PatternVec matchingPatterns;

  // Filter patterns with busyNodes.
  for (auto &busynode : busyNodes)
  {
    patterns.erase(std::remove_if(patterns.begin(), patterns.end(),
                                  [busynode](auto &pattern) { return std::find(pattern.begin(), pattern.end(), busynode) != pattern.end(); }),
                   patterns.end());
  }

  // Generate permutations of each of patterns
  for (auto pattern : patterns)
  {
    sort(pattern.begin(), pattern.end());
    do
    {
      matchingPatterns.emplace_back(pattern);
    } while (next_permutation(pattern.begin(), pattern.end()));
  }
  // for (auto pattern : matchingPatterns)
  // {
  //   logging(pattern);
  // }
  return matchingPatterns;
}

EdgeList getEdges(Pattern pattern, std::string topology, bool nvlinksOnly = false)
{
  EdgeList elist;
  if (topology == "ring")
  {
    uint32_t prev = pattern[0];
    for (size_t i = 1; i < pattern.size(); i++)
    {
      elist.push_back(std::make_pair(prev, pattern[i]));
      prev = pattern[i];
    }
    elist.push_back(std::make_pair(prev, pattern[0]));
  }
  if (topology == "all")
  {
    for (size_t i = 0; i < pattern.size() - 1; i++)
    {
      for (size_t j = (i + 1); j < pattern.size(); j++)
      {
        if (nvlinksOnly)
        {
          if (!bwmap[pattern[i]][pattern[j]].isPCIe())
          {
            elist.push_back(std::make_pair(pattern[i], pattern[j]));
          }
        }
        else
        {
          elist.push_back(std::make_pair(pattern[i], pattern[j]));
        }
      }
    }
  }
  logging("Edges");
  logging(elist);
  return elist;
}

uint32_t getLastScore(Pattern pattern, std::string topology, bool nvlinksOnly = false)
{
  uint32_t lastScore = 0;
  EdgeList elist = getEdges(pattern, topology, nvlinksOnly);
  for (auto &edge : elist)
  {
    lastScore += (bwmap[edge.first][edge.second]).bw;
  }
  return lastScore;
}

uint32_t getLastScoreWithRoute(Pattern pattern, std::string topology)
{
  uint32_t lastScore = 0;
  EdgeList elist = getEdges(pattern, topology);
  for (auto &edge : elist)
  {
    auto conn = bwmap[edge.first][edge.second];
    if (conn.isPCIe() && routeBWmap.size())
    {
      lastScore += routeBWmap[edge.first][edge.second];
    }
    else
    {
      lastScore += conn.bw;
    }
  }
  return lastScore;
}

uint32_t getPreservationScore(Pattern pattern)
{
  uint32_t pScore = 0;

  auto remainingNodes = hwTopo.v_list();
  erase(remainingNodes, busyNodes);
  erase(remainingNodes, pattern);

  if (remainingNodes.size())
  {
    logging(remainingNodes);
    pScore = getLastScore(remainingNodes, "all", true);
  }

  return pScore;
}

void updateFragScore(Allocation &alloc)
{
  alloc.fragScore = 1 - (static_cast<double>(alloc.lastScore) / (static_cast<double>(idealLastScore[alloc.pattern.size()])));
}


#endif
