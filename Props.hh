#ifndef PROPS_H
#define PROPS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"
#include "GpuTopology.hh"

using Pattern = std::vector<uint32_t>;
using EdgeList = std::list<std::pair<uint32_t, uint32_t>>;
using PatternVec = std::vector<Pattern>;
using Nodes = Pattern;

SmallGraph hwTopo;
BwMap bwmap;
RouteBWmap routeBWmap;
std::map<uint32_t, uint32_t> idealLastScore;

// NOTE(Kiran): Check if this is necessary.
uint32_t getTotalLastScore(BwMap bwmap)
{
  uint32_t totalLscore = 0;
  for (auto &outer : bwmap)
  {
    for (auto &inner : outer.second)
    {
      if (!inner.second.isPCIe())
      {
        totalLscore += inner.second.bw;
      }
    }
  }
  return totalLscore;
}

struct GpuSystem
{
  SmallGraph topology;
  BwMap bwmap;
  RouteBWmap routeBWmap;
  std::map<uint32_t, uint32_t> idealLastScore;
  uint32_t numGpus;
  std::string name;

  GpuSystem(SmallGraph topo, BwMap bmap, RouteBWmap rmap, uint32_t num, std::string arch)
  {
    name = arch;
    topology = topo;
    bwmap = bmap;
    routeBWmap = rmap;
    numGpus = num;
    idealLastScore = getIdealLastScore(arch);
  }

  GpuSystem(std::string arch)
  {
    name = arch;
    if ((arch == "dgx-v") || (arch == "dgx-p"))
    {
      topology = cubemesh();
    }
    else if (arch == "summit")
    {
      topology = summitmesh();
    }
    else if (arch == "torus-2d")
    {
      topology = torus2dMesh();
    }
    else if (arch == "mesh")
    {
      topology = cube16Mesh();
    }
    routeBWmap = getRouteBWmap(arch);
    bwmap = getBwMat(arch);
    numGpus = getNumGpusPerNode(arch);
    idealLastScore = getIdealLastScore(arch);
  }
};

struct Allocation
{
  Pattern pattern;
  uint32_t lastScore;
  uint32_t preserveScore;
  double normLastScore;
  // double predictedBW;
  Nodes vertices;
  Nodes antiVertices;
  EdgeList edges;
  EdgeList antiEdges;
  uint32_t numPCIeLinks;
  uint32_t numSingleNVLinks;
  uint32_t numDoubleNVLinks;

 private:
  uint32_t totalNumLinks = numPCIeLinks + numSingleNVLinks + numDoubleNVLinks;
  uint32_t numNVLinks = numSingleNVLinks + numDoubleNVLinks;

public:
  // TODO(kiran): Is this necessary?
  uint32_t getTotalNumLinks()
  {
    return(numPCIeLinks + numSingleNVLinks + numDoubleNVLinks);
  }

  double getLinkRatio()
  {
    if (getTotalNumLinks() == 0)
    {
      return 0;
    }
    else
    {
      return (static_cast<double>(numPCIeLinks) / static_cast<double>(getTotalNumLinks()));
    }
  }

  uint32_t getNumPCIeLinks()
  {
    return numPCIeLinks;
  }

  uint32_t getNumNVLinks()
  {
    return numNVLinks;
  }

  double getPredictedBW()
  {
    double x1 = static_cast<double>(numDoubleNVLinks);
    double x2 = static_cast<double>(numSingleNVLinks);
    double x3 = static_cast<double>(numPCIeLinks);
    double linear = 16.39670455 * x1 + 4.536821878 * x2 + 1.55623404 * x3;
    double inverseLinear = (-20.69484581) * 1 / (x1 + 1) + (-9.467461958) * 1 / (x2 + 1) + 7.615106783 * 1 / (x3 + 1);
    double pairwise = (-7.973335727) * x1 * x2 + 12.73323772 * x2 * x3 + (-4.195655221) * x1 * x3;
    double inversePairwise = (-8.413419363) * 1 / (x1 * x2 + 1) + 62.85125807 * 1 / (x2 * x3 + 1) + 27.41832588 * 1 / (x1 * x3 + 1);
    double triplet = (-5.114108079) * x1 * x2 * x3;
    double inverseTriplet = (-46.97390071) * 1 / ( x1 * x2 * x3 + 1);

    return (linear + inverseLinear + pairwise + inversePairwise + triplet + inverseTriplet);
  }
};

struct JobItem
{
  uint32_t numGpus;
  uint32_t id;
  int pid; // Used to store process ID in realRun.
  std::string topology;
  std::string taskToRun;
  std::vector<SmallGraph> pattern;
  std::vector<uint32_t> schedGPUs; //sched nodes maybe?.
  long int arvlTime;  // Time to move from list to queue.
  long int srvcTime;  // Time to finish a running job (Used in simulation).
  bool bwSensitive;
  Allocation alloc;
  long int startTime; // Time from Queue to Job Scheduled.
  long int endTime;   // Time from Scheduled to Finished.
  long int execTime;  // EndTime - startTime.
  long int queueTime; // startTime - arvlTime.

  JobItem()
  {
  }

  JobItem(std::list<std::string> args, int jid)
  {
    std::list<std::string>::iterator argsIt = args.begin();
    // Advance the iterator by 2 positions,
    numGpus = boost::lexical_cast<int>(*argsIt);
    // pattern = reinterpret_cast<std::string>(args[1]);
    topology = *(++argsIt);
    arvlTime = boost::lexical_cast<long int>(*(++argsIt));
    srvcTime = boost::lexical_cast<long int>(*(++argsIt));
    bwSensitive = boost::lexical_cast<bool>(*(++argsIt));
    taskToRun = *(++argsIt); // NOTE(Kiran): This is to be only used in MgapRealRun
    id = jid; // Size of the jobList at the time of creating this object.
    pid = -1; // Default to errNo. Write appropriate PID when necessary.

    // Note: Unsure if this can check anti-edges.
    if (topology == "ring")
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

  int getId() const
  {
    return id;
  }

  bool operator==(const JobItem &a) const
  {
    return getId() == a.getId();
  }
};

using JobVec = std::vector<JobItem>;

extern JobVec jobList; // Read from file.

void logging(std::string str)
{
  std::cout << "LOG: " << str << std::endl;
}

template <typename T>
void logging(std::vector<T> vec)
{
  std::string str;
  std::for_each(vec.begin(), vec.end(), [&](T elem) { str += std::to_string(elem) + " "; });
  std::cout << "LOG: " << str << std::endl;
}

template <typename T>
void logging(std::list<std::pair<T,T>> vec)
{
  std::for_each(vec.begin(), vec.end(), [&](std::pair<T, T> elem) { std::cout << elem.first << ": " << elem.second << std::endl; });
}

void createLogFile(std::string logFilename)
{
  std::ofstream outFile;
  outFile.open(logFilename);
  outFile << "ID startTime endTime queueTime execTime lastScore normLastScore bwSensitive numGpus linkRatio predictedBW nets schedGpus\n";
  outFile.close();
}

void logresult(JobItem job, std::string logFilename)
{
  std::ofstream outFile;

  outFile.open(logFilename, std::ios_base::app); // append instead of overwrite

  std::string str;
  str = std::to_string(job.id);
  str += " " + std::to_string(job.startTime);
  str += " " + std::to_string(job.endTime);
  str += " " + std::to_string(job.queueTime);
  str += " " + std::to_string(job.execTime);
  str += " " + std::to_string(job.alloc.lastScore);
  str += " " + std::to_string(job.alloc.normLastScore);
  str += " " + std::to_string(job.bwSensitive ? 1 : 0);
  str += " " + std::to_string(job.numGpus);
  str += " " + std::to_string(job.alloc.getLinkRatio());
  str += " " + std::to_string(job.alloc.getPredictedBW());
  str += " " + job.taskToRun;
  str += " ";
  for (auto &node : job.schedGPUs)
  {
    str += std::to_string(node) + ",";
  }
  str += "\n";
  outFile << str;
  outFile.close();
}

#endif
