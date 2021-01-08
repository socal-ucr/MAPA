#ifndef PROPS_H
#define PROPS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"

// TODO(kiran): Implement an enum for LOGLEVEL.
constexpr int LOGLEVEL = 1;

using Pattern = std::vector<uint32_t>;
using EdgeList = std::list<std::pair<uint32_t, uint32_t>>;
using PatternVec = std::vector<Pattern>;
using Nodes = Pattern;
struct Allocation
{
  Pattern pattern;
  uint32_t lastScore;
  double fragScore;
  Nodes vertices;
  Nodes antiVertices;
  EdgeList edges;
  EdgeList antiEdges;
};

struct JobItem
{
  uint32_t numGpus;
  uint32_t id;
  std::string topology;
  std::string taskToRun;
  std::vector<SmallGraph> pattern;
  std::vector<uint32_t> schedGPUs; //sched nodes maybe?.
  uint32_t arvlTime; // Time to move from list to queue.
  uint32_t srvcTime;
  bool bwSensitive;
  Allocation alloc;
  uint32_t startTime; // Time from Queue to Job Scheduled.
  uint32_t endTime;   // Time from Scheduled to Finished.
  uint32_t runtime;   // EndTime - arvlTime.

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
    arvlTime = boost::lexical_cast<uint32_t>(*(++argsIt));
    srvcTime = boost::lexical_cast<uint32_t>(*(++argsIt));
    bwSensitive = boost::lexical_cast<bool>(*(++argsIt));
    taskToRun = *(++argsIt); // NOTE(Kiran): This is to be only used in MgapRealRun
    id = jid; // Size of the jobList at the time of creating this object.

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

SmallGraph currTopo;
SmallGraph hwTopo;
BwMap bwmap;

void logging(std::string str, int level)
{
  if (level > LOGLEVEL)
  {
    std::cout << "LOG: " << str << std::endl;
  }
}

template <typename T>
void logging(std::vector<T> vec, int level)
{
  if (level > LOGLEVEL)
  {
    std::string str;
    std::for_each(vec.begin(), vec.end(), [&](T elem) { str += std::to_string(elem); });
    std::cout << "LOG: " << str << std::endl;
  }
}

template <typename T>
void logging(std::list<std::pair<T,T>> vec, int level)
{
  if (level > LOGLEVEL)
  {
    std::for_each(vec.begin(), vec.end(), [&](std::pair<T, T> elem) { std::cout << elem.first << ":" << elem.second << std::endl; });
  }
}

void logresults(JobVec vec, int level, std::string logFilename)
{
  // std::cout << "ID startTime endTime Allocated schedGpus" << std::endl;
  if (level > LOGLEVEL)
  {
    std::ofstream outFile;
    outFile.open(logFilename);
    outFile << "ID startTime endTime lastScore fragScore bwSensitive numGpus schedGpus\n";
    for (auto &elem : vec)
    {
      std::string str;
      str = std::to_string(elem.id);
      str += " " + std::to_string(elem.startTime);
      str += " " + std::to_string(elem.endTime);
      str += " " + std::to_string(elem.alloc.lastScore);
      str += " " + std::to_string(elem.alloc.fragScore);
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
}

#endif