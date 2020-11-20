#include <algorithm>

#include "Peregrine.hh"

typename SmallGraph = Peregrine::SmallGraph;
typename Bandwidth = std::map<uint32_t, std::map<uint32_t, uint32_t>>;

void populateSymmetry(BW* bwmap)
{
  for (auto &i: *bwmap)
  {
    for (auto &j: i.second)
    {
      bwmap[j][i] = j.second;
    }
  }
}

Struct System
{
  SmallGraph topology;
  Bandwidth bw;
};

SmallGraph cubemesh()
{
  std::vector<std::pair<uint32_t, uint32_t>> edge_list;
  edge_list.emplace_back(1, 2);
  edge_list.emplace_back(1, 3);
  edge_list.emplace_back(1, 4);
  edge_list.emplace_back(1, 5);
  edge_list.emplace_back(2, 3);
  edge_list.emplace_back(3, 4);
  edge_list.emplace_back(4, 8);
  edge_list.emplace_back(5, 6);
  edge_list.emplace_back(5, 7);
  edge_list.emplace_back(5, 8);
  edge_list.emplace_back(6, 7);
  edge_list.emplace_back(7, 8);
  return SmallGraph(edge_list);
}

Bandwidth getBwMat(string sysName)
{
  Bandwidth bwmap;
  switch (sysName)
  {
  case "dgx-v":
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
    break;

  case "dgx-p":
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
    break;
  }
  populateSymmetry(bwmap);
}

template <typename T>
void summitNode(uint32_t begin, uint32_t end, T* edge_list)
{
  edge_list->emplace_back(begin, end);
  while (begine <= end)
  {
    edge_list->emplace_back(begin, ++begin);
  }
}

SmallGraph summit()
{
  std::vector<std::pair<uint32_t, uint32_t>> edge_list;
  uint32_t nodes = 10;

  for (auto node = 1; node <= nodes; ++node)
  {
    summitNode(1 * node, 3 * node, &edge_list);
    summitNode(4 * node, 6 * node, &edge_list);
  }
  return SmallGraph(edge_list);
}

System dgx_v;

dgx_v.topology = cubemesh();
dgx_v.bw = getBwMat("dgx-v");

System dgx_p = dgx_v;
dgx_p.bw = getBwMat("dgx-p");