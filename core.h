#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"

#include "GpuTopology.h"

using Pattern = std::vector<uint32_t>;
using EdgeList = std::list<std::pair<uint32_t, uint32_t>>;
using PatternVec = std::vector<Pattern>;
using Nodes = Pattern;
struct Allocation
{
  Pattern pattern;
  uint32_t lastScore;
};

GpuSystem dgx_v(cubemesh(), getBwMat("dgx-v"));

SmallGraph currTopo = dgx_v.topology;
SmallGraph hwTopo = currTopo;
BwMap bwmap;
Nodes busyNodes;

struct AllocScore
{
  uint32_t lastScore;
  uint32_t fragScore;
  Nodes vertices;
  Nodes antiVertices;
  EdgeList edges;
  EdgeList antiEdges;
};

struct JobItem
{
  int numGpus;
  std::string topology;
  std::vector<SmallGraph> pattern;
  std::vector<uint32_t> schedGPUs;
  uint32_t arvlTime; // Time to move from list to queue.
  uint32_t srvcTime;
  bool bwSensitive;
  AllocScore score;
  uint32_t startTime; // Time from Queue to Job Scheduled.
  uint32_t endTime; // Time from Scheduled to Finished.
  uint32_t runtime; // EndTime - arvlTime.

  JobItem(std::list<std::string> args)
  {
    std::list<std::string>::iterator argsIt = args.begin();
    // Advance the iterator by 2 positions,
    numGpus = boost::lexical_cast<int>(*argsIt);
    // pattern = reinterpret_cast<std::string>(args[1]);
    topology = *(++argsIt);
    arvlTime = boost::lexical_cast<uint32_t>(*(++argsIt));
    srvcTime = boost::lexical_cast<uint32_t>(*(++argsIt));
    bwSensitive = boost::lexical_cast<bool>(*(++argsIt));

    // Note: Unsure if this can check anti-edges.
    if(topology == "ring")
    {
      pattern.emplace_back(Peregrine::PatternGenerator::ring(numGpus));
    }
    else if (topology == "clique")
    {
      pattern.emplace_back(Peregrine::PatternGenerator::clique(numGpus));
    }
    else if (topology == "star")
    {
      pattern.emplace_back(Peregrine::PatternGenerator::star(numGpus));
    }
  }
};

using JobVec = std::vector<JobItem*>;
JobVec jobList;
JobVec jobQueue;
JobVec jobScheduled;
JobVec jobFinished;

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
    JobItem job(j);
    jobList.emplace_back(&job);
  }
}

// TODO(kiran): add and remove nodes might not be necessary if we are filtering patterns.
void removeNodes(Pattern pattern)
{
  Nodes neighbours;
  for (auto &node : pattern)
  {
    neighbours = currTopo.get_neighbours(node);
    for (auto& neighbour: neighbours)
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

bool isFinished(JobItem* job, uint32_t cycles)
{
  uint32_t currTime;
  currTime = job->startTime + job->srvcTime;
  job->endTime = cycles;
  return (cycles == currTime);
}

template <typename Vec, typename T>
void erase(Vec& sourceVec, T item)
{
  sourceVec.erase(std::remove(sourceVec.begin(), sourceVec.end(), item), sourceVec.end());
}

template <typename Vec, typename T>
void moveItem(Vec& destVec, Vec sourceVec, T item)
{
  destVec.emplace_back(item);
  erase(sourceVec, item);
}

Allocation largestLastScore(PatternVec patterns, std::string topology)
{
  Allocation alloc;

  for (auto &pattern : patterns)
  {
    uint32_t currlastScore = getLastScore(pattern, topology);
    if (alloc.lastScore < currlastScore)
    {
      alloc.pattern = pattern;
      alloc.lastScore = currlastScore;
    }
  }
  return alloc;
}

void findPatterns(SmallGraph hwTopo, std::vector<SmallGraph> appTopo)
{
  using namespace Peregrine;
  size_t nthreads = 1;
  std::vector<uint32_t> testingVec;
  const auto callback = [](auto &&handle, auto &&match)
  {
    handle.map(match.pattern, 1);
    utils::store_pattern(match.mapping);
  };
  auto results = match<Pattern, uint64_t, ON_THE_FLY, UNSTOPPABLE>(hwTopo, appTopo, nthreads, callback);
}

Allocation choosePattern(PatternVec patterns, std::string topology)
{
  // filter patterns.
  // patterns.remove_if(
  //   [](auto &pattern) {
  //     for (auto& node: busyNodes)
  //     {
  //       if (std::find(pattern.begin(), pattern.end(), node) != pattern.end())
  //       {
  //         return true;
  //       }
  //     }
  //     return false;
  //   }
  // );

  // Policy-1: Choose the one with highest LAST score.
  if (!patterns.empty())
  {
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

  readJobFile(jobsFilename);

  uint32_t cycles = 0;

  while (!jobList.empty() || !jobQueue.empty() || !jobScheduled.empty())
  {
    if (!jobScheduled.empty())
    {
      for (auto& job: jobScheduled)
      {
        if (isFinished(job, cycles))
        {
          moveItem(jobFinished, jobScheduled, job);
          addNodes(job->schedGPUs);
          for (auto &node : job->schedGPUs)
          {
            erase(busyNodes, node);
          }
        }
      }
    }
    for (auto i = 0; jobList[i]->arvlTime == cycles; ++i)
    {
      jobQueue.emplace_back(jobList[i]);
    }
    for (auto& job : jobQueue)
    {
      findPatterns(currTopo, job->pattern);
      Allocation alloc = choosePattern(utils::foundPatterns, job->topology);
      if (!alloc.pattern.empty())
      {
        removeNodes(alloc.pattern);
        job->startTime = cycles;
        // add busy nodes.
        for (auto& node : alloc.pattern)
        {
          busyNodes.push_back(node);
          job->schedGPUs.push_back(node);
        }
        job->startTime = cycles;
        moveItem(jobScheduled, jobQueue, job);
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
