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
  Nodes vertices;
  Nodes antiVertices;
  EdgeList edges;
  EdgeList antiEdges;
  uint32_t numPCIeLinks;
  uint32_t numNVLinks;
  uint32_t totalNumLinks;

  double getLinkRatio()
  {
    return (reinterpret_cast<double>(numPCIeLinks) / reinterpret_cast<double>(totalNumLinks));
  }

  uint32_t getNumPCIeLinks()
  {
    return numPCIeLinks;
  }

  uint32_t getNumNVLinks()
  {
    return numNVLinks;
  }

  uint32_t getTotalNumLinks()
  {
    return totalNumLinks;
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
  outFile << "ID startTime endTime queueTime execTime lastScore normLastScore bwSensitive numGpus nets schedGpus\n";
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

//TODO(Kiran): logresult() makes this function obsolete. Purge it safely.
void logresults(JobVec vec, std::string logFilename)
{
  std::ofstream outFile;
  outFile.open(logFilename);
  outFile << "ID startTime endTime lastScore normLastScore bwSensitive numGpus schedGpus\n";
  for (auto &elem : vec)
  {
    std::string str;
    str = std::to_string(elem.id);
    str += " " + std::to_string(elem.startTime);
    str += " " + std::to_string(elem.endTime);
    str += " " + std::to_string(elem.queueTime);
    str += " " + std::to_string(elem.execTime);
    str += " " + std::to_string(elem.alloc.lastScore);
    str += " " + std::to_string(elem.alloc.normLastScore);
    str += " " + std::to_string(elem.bwSensitive?1:0);
    str += " " + std::to_string(elem.numGpus);
    str += " ";
    for (auto &node : elem.schedGPUs)
    {
      str += std::to_string(node) + ",";
    }
    str += "\n";
    outFile << str;
  }
  outFile.close();
}

#endif
