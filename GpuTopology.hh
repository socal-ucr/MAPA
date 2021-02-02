#ifndef GPUTOPOLOGY_H
#define GPUTOPOLOGY_H

#include <algorithm>
#include <string>
#include <vector>

#include "Peregrine.hh"

using SmallGraph = Peregrine::SmallGraph;
struct BW
{
  // NOTE(Kiran): WARNING! This datastructure fails to check if the key does not exist.
  uint32_t bw; // Bandwidth of the edge
  enum LinkType {PCIe, NVLink} link;

  BW() {}

  BW (uint32_t bandwidth, LinkType gpuLink)
  {
    bw = bandwidth;
    link = gpuLink;
  }

  bool isPCIe()
  {
    return (link == LinkType::PCIe);
  }
};

using BwMap = std::map<uint32_t, std::map<uint32_t, BW>>;
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
  if (topoName == "torus-2d")
  {
    return 16;
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

SmallGraph torus2dMesh(bool nvlinks = true, bool pcilinks = true)
{
  std::vector<std::pair<uint32_t, uint32_t>> edge_list;
  if (nvlinks)
  {
    edge_list.emplace_back(1, 2);
    edge_list.emplace_back(1, 4);
    edge_list.emplace_back(1, 13);
    edge_list.emplace_back(2, 3);
    edge_list.emplace_back(2, 6);
    edge_list.emplace_back(2, 14);
    edge_list.emplace_back(3, 4);
    edge_list.emplace_back(3, 7);
    edge_list.emplace_back(3, 15);
    edge_list.emplace_back(4, 8);
    edge_list.emplace_back(4, 16);
    edge_list.emplace_back(5, 6);
    edge_list.emplace_back(5, 9);
    edge_list.emplace_back(5, 8);
    edge_list.emplace_back(6, 7);
    edge_list.emplace_back(6, 10);
    edge_list.emplace_back(7, 8);
    edge_list.emplace_back(7, 11);
    edge_list.emplace_back(8, 12);
    edge_list.emplace_back(9, 10);
    edge_list.emplace_back(9, 13);
    edge_list.emplace_back(9, 12);
    edge_list.emplace_back(10, 11);
    edge_list.emplace_back(10, 14);
    edge_list.emplace_back(11, 12);
    edge_list.emplace_back(11, 15);
    edge_list.emplace_back(12, 16);
    edge_list.emplace_back(13, 14);
    edge_list.emplace_back(13, 16);
    edge_list.emplace_back(14, 15);
    edge_list.emplace_back(15, 16);
  }
  if (pcilinks)
  {
    for (uint32_t i = 1; i <= 15; i++)
    {
      for (uint32_t j = i + 1; j <= 16; j++)
      {
        if (std::find(std::begin(edge_list), std::end(edge_list), std::make_pair(i, j)) == edge_list.end())
        {
          edge_list.emplace_back(i, j);
        }
      }
    }
  }
  return SmallGraph(edge_list);
}

BwMap populateSymmetry(BwMap bwmap)
{
  for (auto i : bwmap)
  {
    for (auto j : i.second)
    {
      bwmap[j.first][i.first] = BW((j.second).bw, (j.second).link);
    }
  }
  return bwmap;
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
      bwmap[4][8] = BW(25, BW::LinkType::NVLink);
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
      bwmap[4][8] = BW(20, BW::LinkType::NVLink);
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
  else if (sysName == "torus-2d")
  {
    if (nvlinks)
    {
      bwmap[1][2] = BW(50, BW::LinkType::NVLink);
      bwmap[1][4] = BW(25, BW::LinkType::NVLink);
      bwmap[1][13] = BW(50, BW::LinkType::NVLink);
      bwmap[2][3] = BW(50, BW::LinkType::NVLink);
      bwmap[2][6] = BW(25, BW::LinkType::NVLink);
      bwmap[2][14] = BW(25, BW::LinkType::NVLink);
      bwmap[3][4] = BW(50, BW::LinkType::NVLink);
      bwmap[3][7] = BW(25, BW::LinkType::NVLink);
      bwmap[3][15] = BW(25, BW::LinkType::NVLink);
      bwmap[4][8] = BW(25, BW::LinkType::NVLink);
      bwmap[4][16] = BW(50, BW::LinkType::NVLink);
      bwmap[5][6] = BW(50, BW::LinkType::NVLink);
      bwmap[5][9] = BW(25, BW::LinkType::NVLink);
      bwmap[5][8] = BW(25, BW::LinkType::NVLink);
      bwmap[6][7] = BW(50, BW::LinkType::NVLink);
      bwmap[6][10] = BW(25, BW::LinkType::NVLink);
      bwmap[7][8] = BW(50, BW::LinkType::NVLink);
      bwmap[7][11] = BW(25, BW::LinkType::NVLink);
      bwmap[8][12] = BW(25, BW::LinkType::NVLink);
      bwmap[9][10] = BW(50, BW::LinkType::NVLink);
      bwmap[9][13] = BW(25, BW::LinkType::NVLink);
      bwmap[9][12] = BW(25, BW::LinkType::NVLink);
      bwmap[10][11] = BW(50, BW::LinkType::NVLink);
      bwmap[10][14] = BW(25, BW::LinkType::NVLink);
      bwmap[11][12] = BW(50, BW::LinkType::NVLink);
      bwmap[11][15] = BW(25, BW::LinkType::NVLink);
      bwmap[12][16] = BW(25, BW::LinkType::NVLink);
      bwmap[13][14] = BW(50, BW::LinkType::NVLink);
      bwmap[13][16] = BW(25, BW::LinkType::NVLink);
      bwmap[14][15] = BW(50, BW::LinkType::NVLink);
      bwmap[15][16] = BW(50, BW::LinkType::NVLink);
    }
    if (pcilinks)
    {
      for (uint32_t i = 1; i <= 15; i++)
      {
        for (uint32_t j = i + 1; j <= 16; j++)
        {
          auto itA = bwmap.find(i);
          auto itB = (itA->second).find(j);
          if (itB == (itA->second).end())
          {
            bwmap[i][j] = BW(10, BW::LinkType::PCIe);
          }
        }
      }
    }
  }
  return populateSymmetry(bwmap);
}

std::map<uint32_t, uint32_t> getIdealLastScore(std::string arch)
{
  std::map<uint32_t, uint32_t> idealLscore;
  if (arch == "dgx-v")
  {
    idealLscore[2] = 100;
    idealLscore[3] = 125;
    idealLscore[4] = 175;
    idealLscore[5] = 210;
    idealLscore[6] = 275;
    idealLscore[7] = 325;
    idealLscore[8] = 400;
  }
  if (arch == "dgx-p")
  {
    idealLscore[2] = 40;
    idealLscore[3] = 60;
    idealLscore[4] = 80;
    idealLscore[5] = 90;
    idealLscore[6] = 120;
    idealLscore[7] = 130;
    idealLscore[8] = 160;
  }
  else if (arch == "torus-2d")
  {
    idealLscore[2] = 100;
    idealLscore[3] = 110;
    idealLscore[4] = 175;
    idealLscore[5] = 210;
    idealLscore[6] = 260;
  }
  else if (arch == "summit")
  {
    idealLscore[2] = 100;
    idealLscore[3] = 150;
    idealLscore[4] = 170;
    idealLscore[5] = 220;
    idealLscore[6] = 320;
  }
  return idealLscore;
}

RouteBWmap getRouteBWmap(std::string sysName)
{
  RouteBWmap rmap;
  if (sysName == "dgx-v")
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
  if (sysName == "dgx-p")
  {
    rmap[1][6] = 20;
    rmap[1][7] = 20;
    rmap[1][8] = 20;
    rmap[2][5] = 20;
    rmap[2][7] = 20;
    rmap[2][8] = 20;
    rmap[3][5] = 20;
    rmap[3][6] = 20;
    rmap[3][8] = 20;
    rmap[4][5] = 20;
    rmap[4][6] = 20;
    rmap[4][7] = 20;
    rmap[5][2] = 20;
    rmap[5][3] = 20;
    rmap[5][4] = 20;
    rmap[6][4] = 20;
    rmap[6][3] = 20;
    rmap[6][1] = 20;
    rmap[7][1] = 20;
    rmap[7][2] = 20;
    rmap[7][4] = 20;
    rmap[8][1] = 20;
    rmap[8][2] = 20;
    rmap[8][3] = 20;
  }
  if (sysName == "torus-2d")
  {
    BwMap bmap = getBwMat(sysName);
    for (auto i = 1; i <= 16; ++i)
    {
      for (auto j = 1; j <= 16; j++)
      {
        if (bmap[i][j].isPCIe())
        {
          // TODO(Kiran) #2 : Double Check if the assumption is correct.
          rmap[i][j] = 25;
        }
      }
    }
  }
  return rmap;
}

#endif
