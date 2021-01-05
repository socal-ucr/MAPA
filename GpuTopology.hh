#ifndef GPUTOPOLOGY_H
#define GPUTOPOLOGY_H

#include <algorithm>
#include <string>
#include <vector>

#include "Peregrine.hh"

using SmallGraph = Peregrine::SmallGraph;
using BwMap = std::map<uint32_t, std::map<uint32_t, uint32_t>>;
using numGpuMat = std::map<std::string, uint32_t>;

uint32_t getNumGpusPerNode(std::string topoName)
{
  if (topoName == "dgx-v")
  {
    return 8;
  }
  else if (topoName == "dgx-p")
  {
    return 8;
  }
  // if (topoName == "summit")
  // {
  //   return 6;
  // }
  return 0;
}

SmallGraph cubemesh()
{
  std::vector<std::pair<uint32_t, uint32_t>> edge_list;
  edge_list.emplace_back(1, 2);
  edge_list.emplace_back(1, 3);
  edge_list.emplace_back(1, 4);
  edge_list.emplace_back(1, 5);
  edge_list.emplace_back(2, 3);
  edge_list.emplace_back(2, 4);
  edge_list.emplace_back(2, 6);
  edge_list.emplace_back(3, 4);
  edge_list.emplace_back(3, 7);
  edge_list.emplace_back(4, 8);
  edge_list.emplace_back(5, 6);
  edge_list.emplace_back(5, 7);
  edge_list.emplace_back(5, 8);
  edge_list.emplace_back(6, 7);
  edge_list.emplace_back(6, 8);
  edge_list.emplace_back(7, 8);
  return SmallGraph(edge_list);
}
// template <typename T>
// void summitNode(uint32_t begin, uint32_t end, T *edge_list)
// {
//   edge_list->emplace_back(begin, end);
//   while (begin <= end)
//   {
//     edge_list->emplace_back(begin, ++begin);
//   }
// }

// SmallGraph summitTopo()
// {
//   std::vector<std::pair<uint32_t, uint32_t>> edge_list;
//   uint32_t nodes = 10;

//   for (auto node = 1; node <= nodes; ++node)
//   {
//     summitNode(1 * node, 3 * node, &edge_list);
//     summitNode(4 * node, 6 * node, &edge_list);
//   }
//   return SmallGraph(edge_list);
// }

BwMap populateSymmetry(BwMap bwmap)
{
  for (auto &i : bwmap)
  {
    for (auto &j : i.second)
    {
      bwmap[j.first][i.first] = j.second;
    }
  }
  return bwmap;
}

BwMap getBwMat(std::string sysName)
{
  BwMap bwmap;
  
  if (sysName == "dgx-v")
  {
    bwmap[1][2] = 25;
    bwmap[1][3] = 25;
    bwmap[1][4] = 50;
    bwmap[1][5] = 50;
    bwmap[2][3] = 50;
    bwmap[2][4] = 25;
    bwmap[2][6] = 50;
    bwmap[3][4] = 50;
    bwmap[3][7] = 25;
    bwmap[5][8] = 50;
    bwmap[5][6] = 25;
    bwmap[5][7] = 25;
    bwmap[6][7] = 50;
    bwmap[6][8] = 25;
    bwmap[7][8] = 50;
  }

  else if (sysName == "dgx-p")
  {
    bwmap[1][2] = 20;
    bwmap[1][3] = 20;
    bwmap[1][4] = 20;
    bwmap[1][5] = 20;
    bwmap[2][3] = 20;
    bwmap[2][4] = 20;
    bwmap[2][6] = 20;
    bwmap[3][4] = 20;
    bwmap[3][7] = 20;
    bwmap[5][8] = 20;
    bwmap[5][6] = 20;
    bwmap[5][7] = 20;
    bwmap[6][7] = 20;
    bwmap[6][8] = 20;
    bwmap[7][8] = 20;
  }
  // else if (sysName == "summit")
  // {
  //   bwmap[1][2] = 20;
  //   bwmap[2][3] = 20;
  //   bwmap[3][1] = 20;
  //   bwmap[4][5] = 20;
  //   bwmap[5][6] = 20;
  //   bwmap[6][4] = 20;
  // }
  return populateSymmetry(bwmap);
}

uint32_t getIdealLastScore(BwMap bwmap)
{
  uint32_t idealLastScore = 0;
  for (auto &outer : bwmap)
  {
    for (auto &inner : outer.second)
    {
      if (idealLastScore < inner.second)
      {
        idealLastScore = inner.second;
      }
    }
  }
  return idealLastScore;
}

struct GpuSystem
{
  SmallGraph topology;
  BwMap bwmap;
  uint32_t idealLastScore;
  uint32_t numGpus;

  GpuSystem(SmallGraph topo, BwMap bmap, uint32_t num)
  {
    topology = topo;
    bwmap = bmap;
    numGpus = num;
    idealLastScore = getIdealLastScore(bwmap);
  }

  GpuSystem(std::string arch)
  {
    topology = cubemesh();
    bwmap = getBwMat(arch);
    numGpus = getNumGpusPerNode(arch);
    idealLastScore = getIdealLastScore(bwmap);
  }
};

#endif