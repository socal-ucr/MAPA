#include <iostream>
#include <fstream>
#include <vector>

#include "Peregrine.hh"
#include "GpuTopology.h"
#include "MinePattern.hh"

typename NodeList = std::vector<uint32_t>;

SmallGraph origTopo;
SmallGraph currTopo;

origTopo = dgx_v;
currTopo = origTopo;

struct JobItem
{
  int numGpus;
  SmallGraph pattern;
  std::vector<int> schedGPUs;
  int arvlTime;
  int srvcTime;
  bool bwSensitive;
}

typename JobList = std::vector<JobItem>;
typename JobQueue = std::vector<JobItem>;
typename JobScheduled = std::vector<JobItem>;

JobList readJobFile(str fname)
{
}

void removeNodes(NodeList nodes)
{
  NodeList neighList;
  for (auto &node : nodes)
  {
    neighList = currTopo.get_neighbours(n);
    for (auto& neigh: neighList)
    {
      currTopo.remove_edge(node, neigh);
    }
  }
}

void addNodes(NodeList nodes)
{
  NodeList neighList;
  for (auto &node : nodes)
  {
    neighList = currTopo.get_neighbours(n);
    for (auto &neigh : neighList)
    {
      currTopo.add_edge(node, neigh);
    }
  }
}

int getLastScore(NodeList nodes)
{
  NodeList neighList;
  for (auto &node : nodes)
  {
    neighList = currTopo.get_neighbours(n);
    for (auto &neigh : neighList)
    {
      currTopo.add_edge(node, neigh);
    }
  }
}

size_t schedJobs(JobList jobList, SmallGraph currTopo)
{
  JobQueue jq;
  JobScheduled js;
  size_t cycles = 0;
  while (!jobList.empty())
  {
    for (auto i = 0; jobList[i].arvlTime == cycles; ++i)
    {
      
    }
  }
  
  return cycles;
}

void simulate(string jobsFilename)
{
  JobList jobs;
  jobs = readJobFile(jobsFilename);
  return schdJobs(jobs, currTopo);
}

