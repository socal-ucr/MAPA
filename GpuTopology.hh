#ifndef GPUTOPOLOGY_H
#define GPUTOPOLOGY_H

#include <algorithm>
#include <string>
#include <vector>

#include "Peregrine.hh"

using SmallGraph = Peregrine::SmallGraph;
struct BW
{
  uint32_t bw; // Bandwidth of the edge
  enum LinkType {PCIe, NVLink} link;

  BW() {}

  BW operator=(const BW& b)
  {
    bw = b.bw;
    link = b.link;
    return *this;
  }

  BW(uint32_t bandwidth, LinkType gpuLink)
  {
    bw = bandwidth;
    link = gpuLink;
  }

  bool isPCIe()
  {
    return (link == LinkType::PCIe);
  }
};

using BwMap = std::map<uint32_t, std::map<uint32_t, struct BW>>;
using RouteBWmap = std::map<uint32_t, std::map<uint32_t, uint32_t>>;
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
    edge_list.emplace_back(1, 6);
    edge_list.emplace_back(1, 7);
    edge_list.emplace_back(1, 8);
    edge_list.emplace_back(2, 5);
    edge_list.emplace_back(2, 7);
    edge_list.emplace_back(2, 8);
    edge_list.emplace_back(3, 5);
    edge_list.emplace_back(3, 6);
    edge_list.emplace_back(3, 8);
    edge_list.emplace_back(4, 5);
    edge_list.emplace_back(4, 6);
    edge_list.emplace_back(4, 7);
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

RouteBWmap getRouteBWmap(std::string sysName)
{
  RouteBWmap rmap;
  if ((sysName == "dgx-v") || (sysName == "dgx-p"))
  {
    rmap[1][6] = 50;
    rmap[1][7] = 25;
    rmap[1][8] = 25;
    rmap[2][5] = 25;
    rmap[2][7] = 25;
    rmap[2][8] = 25;
    rmap[3][5] = 25;
    rmap[3][6] = 50;
    rmap[3][8] = 25;
    rmap[4][5] = 50;
    rmap[4][6] = 25;
    rmap[4][7] = 25;
    rmap[5][2] = 25;
    rmap[5][3] = 25;
    rmap[5][4] = 25;
    rmap[6][4] = 25;
    rmap[6][3] = 50;
    rmap[6][1] = 25;
    rmap[7][1] = 25;
    rmap[7][2] = 25;
    rmap[7][4] = 25;
    rmap[8][1] = 50;
    rmap[8][2] = 25;
    rmap[8][3] = 25;
  }
  return rmap;
}

BwMap getBwMat(std::string sysName, bool nvlinks = true, bool pcilinks = true)
{
  BwMap bwmap;
  if (sysName == "dgx-v")
  {
    if (nvlinks)
    {
      bwmap[1][2] = BW(25, BW::LinkType::NVLink);
      bwmap[1][3] = BW(25, BW::LinkType::NVLink);
      bwmap[1][4] = BW(50, BW::LinkType::NVLink);
      bwmap[1][5] = BW(50, BW::LinkType::NVLink);
      bwmap[2][3] = BW(50, BW::LinkType::NVLink);
      bwmap[2][4] = BW(25, BW::LinkType::NVLink);
      bwmap[2][6] = BW(50, BW::LinkType::NVLink);
      bwmap[3][4] = BW(50, BW::LinkType::NVLink);
      bwmap[3][7] = BW(25, BW::LinkType::NVLink);
      bwmap[5][8] = BW(50, BW::LinkType::NVLink);
      bwmap[5][6] = BW(25, BW::LinkType::NVLink);
      bwmap[5][7] = BW(25, BW::LinkType::NVLink);
      bwmap[6][7] = BW(50, BW::LinkType::NVLink);
      bwmap[6][8] = BW(25, BW::LinkType::NVLink);
      bwmap[7][8] = BW(50, BW::LinkType::NVLink);
    }
    if (pcilinks)
    {
      bwmap[1][6] = BW(10, BW::LinkType::PCIe);
      bwmap[1][7] = BW(10, BW::LinkType::PCIe);
      bwmap[1][8] = BW(10, BW::LinkType::PCIe);
      bwmap[2][5] = BW(10, BW::LinkType::PCIe);
      bwmap[2][7] = BW(10, BW::LinkType::PCIe);
      bwmap[2][8] = BW(10, BW::LinkType::PCIe);
      bwmap[3][5] = BW(10, BW::LinkType::PCIe);
      bwmap[3][6] = BW(10, BW::LinkType::PCIe);
      bwmap[3][8] = BW(10, BW::LinkType::PCIe);
      bwmap[4][5] = BW(10, BW::LinkType::PCIe);
      bwmap[4][6] = BW(10, BW::LinkType::PCIe);
      bwmap[4][7] = BW(10, BW::LinkType::PCIe);
    }
  }

  else if (sysName == "dgx-p")
  {
    if (nvlinks)
    {
      bwmap[1][2] = BW(20, BW::LinkType::NVLink);
      bwmap[1][3] = BW(20, BW::LinkType::NVLink);
      bwmap[1][4] = BW(20, BW::LinkType::NVLink);
      bwmap[1][5] = BW(20, BW::LinkType::NVLink);
      bwmap[2][3] = BW(20, BW::LinkType::NVLink);
      bwmap[2][4] = BW(20, BW::LinkType::NVLink);
      bwmap[2][6] = BW(20, BW::LinkType::NVLink);
      bwmap[3][4] = BW(20, BW::LinkType::NVLink);
      bwmap[3][7] = BW(20, BW::LinkType::NVLink);
      bwmap[5][8] = BW(20, BW::LinkType::NVLink);
      bwmap[5][6] = BW(20, BW::LinkType::NVLink);
      bwmap[5][7] = BW(20, BW::LinkType::NVLink);
      bwmap[6][7] = BW(20, BW::LinkType::NVLink);
      bwmap[6][8] = BW(20, BW::LinkType::NVLink);
      bwmap[7][8] = BW(20, BW::LinkType::NVLink);
    }
    if (pcilinks)
    {
      bwmap[1][6] = BW(10, BW::LinkType::PCIe);
      bwmap[1][7] = BW(10, BW::LinkType::PCIe);
      bwmap[1][8] = BW(10, BW::LinkType::PCIe);
      bwmap[2][5] = BW(10, BW::LinkType::PCIe);
      bwmap[2][7] = BW(10, BW::LinkType::PCIe);
      bwmap[2][8] = BW(10, BW::LinkType::PCIe);
      bwmap[3][5] = BW(10, BW::LinkType::PCIe);
      bwmap[3][6] = BW(10, BW::LinkType::PCIe);
      bwmap[3][8] = BW(10, BW::LinkType::PCIe);
      bwmap[4][5] = BW(10, BW::LinkType::PCIe);
      bwmap[4][6] = BW(10, BW::LinkType::PCIe);
      bwmap[4][7] = BW(10, BW::LinkType::PCIe);
    }
  }
  else if (sysName == "summit")
  {
    if (nvlinks)
    {
      bwmap[1][2] = BW(50, BW::LinkType::NVLink);
      bwmap[2][3] = BW(50, BW::LinkType::NVLink);
      bwmap[3][1] = BW(50, BW::LinkType::NVLink);
      bwmap[4][5] = BW(50, BW::LinkType::NVLink);
      bwmap[5][6] = BW(50, BW::LinkType::NVLink);
      bwmap[6][4] = BW(50, BW::LinkType::NVLink);
    }
    if (pcilinks)
    {
      bwmap[1][4] = BW(10, BW::LinkType::PCIe);
      bwmap[1][5] = BW(10, BW::LinkType::PCIe);
      bwmap[1][6] = BW(10, BW::LinkType::PCIe);
      bwmap[2][4] = BW(10, BW::LinkType::PCIe);
      bwmap[2][5] = BW(10, BW::LinkType::PCIe);
      bwmap[2][6] = BW(10, BW::LinkType::PCIe);
      bwmap[3][4] = BW(10, BW::LinkType::PCIe);
      bwmap[3][5] = BW(10, BW::LinkType::PCIe);
      bwmap[3][6] = BW(10, BW::LinkType::PCIe);
    }
  }
  return populateSymmetry(bwmap);
}

#endif
