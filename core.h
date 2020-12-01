#include <iostream>
#include <fstream>
#include <vector>

#include "Peregrine.hh"
#include "GpuTopology.h"
// #include "MinePattern.hh"

using Nodes = std::list<uint32_t>;
using EdgeList = std::list<std::pair<uint32_t, uint32_t>>;
using NodesList = std::list<Nodes>;

System sys = dgx_v
SmallGraph currTopo = dgx_v.topology;
const SmallGraph hwTopo = currTopo;
Nodes busyNodes;

struct AllocScore
{
  uint32_t lastScore;
  uint32_t fragScore;
  Nodes vertices;
  Nodes antiVertices;
  EdgeList edges;
  EdgeList antiEdges;
}

struct JobItem
{
  int numGpus;
  SmallGraph pattern;
  std::vector<int> schedGPUs;
  uint32_t arvlTime; // Time to move from list to queue.
  uint32_t srvcTime;
  bool bwSensitive;
  AllocScore score;
  uint32_t startTime; // Time from Queue to Job Scheduled.
  uint32_t endTime; // Time from Scheduled to Finished.
  uint32_t runtime; // EndTime - arvlTime.

  void insertStartTime(uint32_t t)
  {
    startTime = t;
  }
  
  // void findNode(uint32_t node)
  // {
  //   // for (...) find in 
  // }
};

using JobVec = std::vector<JobItem>;
JobVec jobList;
JobVec jobQueue;
JobVec jobScheduled;
JobVec jobFinished;

void readJobFile(std::string fname)
{
  ifstream jobFile(fname);
  while (getline(jobFile, line))
  {
    std::string delimiter = ",";
    size_t pos = 0;
    std::string<std::string> token(4);
    while ((pos = line.find(delimiter)) != std::string::npos)
    {
      token = line.substr(0, pos);
      std::cout << token << std::endl;
      line.erase(0, pos + delimiter.length());
    }
    std::cout << line << std::endl;
  }
  jobFile.close();
}

// TODO(kiran): add and remove nodes might not be necessary if we are filtering patterns.
void removeNodes(Nodes nodes)
{
  Nodes neighbours;
  for (auto &node : nodes)
  {
    neighbours = currTopo.get_neighbours(node);
    for (auto& neighbour: neighbours)
    {
      currTopo.remove_edge(node, neighbour);
    }
  }
}

void addNodes(Nodes nodes)
{
  Nodes neighbours;
  for (auto &node : nodes)
  {
    neighbours = hwTopo.get_neighbours(node);
    for (auto &neighbour : neighbours)
    {
      currTopo.add_edge(node, neighbour);
    }
  }
}

size_t getLastScore(NodesList possiblePatterns)
{
  size_t lastScore = 0;
  for (auto &pattern : possiblePatterns)
  {
    edgeList = hwTopo.get_edges(pattern);
    for (auto &edge: edgeList)
    {
      lastScore += sys.bwMap[edge.first][edge.second];
    }
  }
}

bool isFinished(JobItem& job, uint32_t cycles)
{
  uint32_t currTime;
  currTime = job.startTime + job.srvcTime;
  job.endTime = cycles;
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

Nodes sort(NodesList patternList)
{
  Nodes select;
  uint32_t lastScore;
  uint32_t selectLastScore;

  for (auto &pattern : patternList)
  {
    lastScore = getLastScore(pattern);
    if (selectLastScore < lastScore)
    {
      select = pattern;
      selectLastScore = lastScore;
    }
  }
  // TODO(kiran): Check if selectLastScore also need to be returned.
  return pattern;
}

Nodes choosePattern(NodesList patternList)
{
  // filter patterns.
  patternList.remove_if(
    [](auto &pattern) {
      for (auto& node: busyNodes)
      {
        if (pattern.contain(node)
        {
          return true;
        }
      }
      return false;
    }
  );
  // for (auto &pattern : patternList)
  // {
  //   for (auto& node : busyNodes) // Check iterator if it is updated.
  //   {
  //     if(pattern.contain(node))
  //     {
  //       patternList.remove(pattern);
  //       return true;
  //     }
  //   }
  //   return false;
  // }

  // Policy-1: Choose the one with highest LAST score.
  return sort(patternList);
}

uint32_t simulate(std::string jobsFilename, System sys)
{
  NodesList possiblePatterns;

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
          // addNodes(job.schedGPUs);
          for (auto &node : job.schedGPUs)
          {
            erase(busyNodes, node);
          }
        }
      }
    }
    for (auto i = 0; jobList[i].arvlTime == cycles; ++i)
    {
      jobQueue.emplace_back(jobList[i]);
    }
    for (auto& job : jobQueue)
    {
      possiblePatterns = MinePattern::search(currTopo, j.pattern);
      allocation = choosePattern(possiblePatterns);
      if (allocation)
      {
        // removeNodes(allocation);
        job.startTime = cycles;
        // add busy nodes.
        for (auto&node : allocation)
        {
          busyNodes.push_back(node);
          job.schedGPUs.push_back(node);
        }
        job.startTime = cycles;
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
