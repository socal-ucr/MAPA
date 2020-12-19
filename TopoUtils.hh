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

extern SmallGraph currTopo;
extern SmallGraph hwTopo;

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

// TODO(kiran): add and remove nodes might not be necessary if we are filtering patterns.
void removeNodes(Pattern pattern)
{
  Nodes neighbours;
  for (auto &node : pattern)
  {
    neighbours = currTopo.get_neighbours(node);
    for (auto &neighbour : neighbours)
    {
      currTopo.remove_edge(node, neighbour);
    }
  }
}

void addNodes(Pattern pattern)
{
  Nodes neighbours;
  for (auto &node : pattern)
  {
    std::cout << "Getting neigh for " << node << std::endl;
    neighbours = hwTopo.get_neighbours(node);
    for (auto &neighbour : neighbours)
    {
      currTopo.add_edge(node, neighbour);
    }
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
  logging("Edges", 1);
  logging(elist, 1);
  return elist;
}

uint32_t getLastScore(Pattern pattern, std::string topology)
{
  uint32_t lastScore = 0;
  EdgeList elist = getEdges(pattern, topology);
  for (auto &edge : elist)
  {
    lastScore += bwmap[edge.first][edge.second];
  }
  return lastScore;
}

#endif