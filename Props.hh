#ifndef PROPS_H
#define PROPS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"

using Pattern = std::vector<uint32_t>;
using EdgeList = std::list<std::pair<uint32_t, uint32_t>>;
using PatternVec = std::vector<Pattern>;
using Nodes = Pattern;

struct Allocation
{
  Pattern pattern;
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
  int id;
  std::string topology;
  std::vector<SmallGraph> pattern;
  std::vector<uint32_t> schedGPUs;
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

  JobItem(std::list<std::string> args, int id)
  {
    std::list<std::string>::iterator argsIt = args.begin();
    // Advance the iterator by 2 positions,
    numGpus = boost::lexical_cast<int>(*argsIt);
    // pattern = reinterpret_cast<std::string>(args[1]);
    topology = *(++argsIt);
    arvlTime = boost::lexical_cast<uint32_t>(*(++argsIt));
    srvcTime = boost::lexical_cast<uint32_t>(*(++argsIt));
    bwSensitive = boost::lexical_cast<bool>(*(++argsIt));
    id = id; // Size of the jobList at the time of creating this object.

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

#endif