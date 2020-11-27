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
    std::cout << s << std::endl;
  }
  jobFile.close();
}

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

template <typename Vec&, typename T>
void moveItem(Vec destVec, Vec sourceVec, T item)
{
  destVec.emplace_back(job);
  sourceVec.erase(std::remove(sourceVec.begin(), sourceVec.end(), job), sourceVec.end());
}

void simulate(string jobsFilename)
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
        updateTopo(allocation);
        job.startTime = cycles;
        moveItem(jobScheduled, jobQueue, job);
      }
    }
    cycles++;
  }
  return cycles;
}

void realRun()
{
}
