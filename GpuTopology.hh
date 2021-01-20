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
  if (topoName == "summit")
  {
    return 6;
  }
  return 0;
}

SmallGraph cubemesh(bool nvlinks = true, bool pcilinks = true)
{
  std::vector<std::pair<uint32_t, uint32_t>> edge_list;
  if (nvlinks)
  {
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
  }
  if (pcilinks)
  {

  }
  return SmallGraph(edge_list);
}

SmallGraph summitmesh(bool nvlinks = true, bool pcilinks = true)
{
  std::vector<std::pair<uint32_t, uint32_t>> edge_list;
  if (nvlinks)
  {
    edge_list.emplace_back(1, 2);
    edge_list.emplace_back(1, 3);
    edge_list.emplace_back(2, 3);
    edge_list.emplace_back(4, 5);
    edge_list.emplace_back(4, 6);
    edge_list.emplace_back(5, 6);
  }
  if (pcilinks)
  {
    edge_list.emplace_back(1, 4);
    edge_list.emplace_back(1, 5);
    edge_list.emplace_back(1, 6);
    edge_list.emplace_back(2, 4);
    edge_list.emplace_back(2, 5);
    edge_list.emplace_back(2, 6);
    edge_list.emplace_back(3, 4);
    edge_list.emplace_back(3, 5);
    edge_list.emplace_back(3, 6);
  }
  return SmallGraph(edge_list);
}

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

BwMap getBwMat(std::string sysName, bool nvlinks = true, bool pcilinks = true)
{
  BwMap bwmap;
  // Add remaining links for preservation score.
  if (sysName == "dgx-v")
  {
    // NVLinks
    if (nvlinks)
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
    if (pcilinks)
    {
      
    }
    // PCIe Links
  }

  else if (sysName == "dgx-p")
  {
    // NVLinks
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
    // PCIe Links
  }
  else if (sysName == "summit")
  {
    if (nvlinks)
    {
      bwmap[1][2] = 20;
      bwmap[2][3] = 20;
      bwmap[3][1] = 20;
      bwmap[4][5] = 20;
      bwmap[5][6] = 20;
      bwmap[6][4] = 20;
    }
    if (pcilinks)
    {
      bwmap[1][4] = 10;
      bwmap[1][5] = 10;
      bwmap[1][6] = 10;
      bwmap[2][4] = 10;
      bwmap[2][5] = 10;
      bwmap[2][6] = 10;
      bwmap[3][4] = 10;
      bwmap[3][5] = 10;
      bwmap[3][6] = 10;
    }
  }
  return populateSymmetry(bwmap);
}

#endif
