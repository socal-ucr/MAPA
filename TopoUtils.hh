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
extern SmallGraph hwTopo;
extern uint32_t idealLastScore;

void readJobFile(std::string fname)
{
  // Format: numGpus, topology, arrivalTime, executionTime, bwSensitivity
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

EdgeList getEdges(Pattern pattern, std::string topology)
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
  }
  logging("Edges");
  logging(elist);
  return elist;
}

uint32_t getLastScore(Pattern pattern, std::string topology)
{
  uint32_t lastScore = 0;
  EdgeList elist = getEdges(pattern, topology);
  for (auto &edge : elist)
  {
    auto bw = bwmap[edge.first][edge.second].bw;
    lastScore += bw;
  }
  return lastScore;
}

void updateFragScore(Allocation& alloc)
{
  alloc.fragScore = 1 - (static_cast<double>(alloc.lastScore) / (static_cast<double>(idealLastScore) * alloc.pattern.size()));
}

// uint32_t getPreservationScore(Pattern pattern, std::string topology)
// {
//   uint32_t preservationScore = 0;
//   return preservationScore;
// }

#endif
