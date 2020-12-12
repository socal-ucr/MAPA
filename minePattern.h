#include <iostream>

#include "core/Options.hh"
#include "core/utils.hh"
#include "core/Graph.hh"
#include "core/PatternGenerator.hh"
#include "core/PatternMatching.hh"
#include "Peregrine.hh"
#include <string>

using namespace Peregrine;

SmallGraph generateCyclic(uint32_t size)
{
  std::vector<std::pair<uint32_t, uint32_t>> edge_list;
  uint32_t init = 1;
  for (uint32_t prev = init, curr = init + 1; curr <= size; prev = curr, ++curr)
  {
    edge_list.emplace_back(prev, curr);
  }
  edge_list.emplace_back(size, init);
  return SmallGraph(edge_list);
}

template <typename T>
void printResults(T results)
{
  for (const auto &[pattern, val] : results)
  {
    std::cout << pattern << " " << val << std::endl;
  }
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << argv[0] << " pattern_size\n";
    exit(0);
  }

  else
  {
    int patternSize = atoi(argv[1]);
    std::vector<Peregrine::SmallGraph> patternsToMine; // App graph

    patternsToMine.emplace_back(generateCyclic(patternSize)); // 4 -> 1 -> 2 -> 3 -> 4

    std::cout << "\nRunning Match test\n"
              << std::endl;

    size_t nthreads = 1;
    // const auto callback = [](auto &&handle, auto &&match) { handle.map(match.pattern, 1); };
    const auto callback = [](auto &&handle, auto &&match) { utils::print_vector(match.mapping); };
    auto results = match<Pattern, uint64_t, ON_THE_FLY, UNSTOPPABLE>(SmallGraph("topoGraph.txt"), patternsToMine, nthreads, callback);
    printResults(results);

    std::cout << "\nRunning Counts test\n"
              << std::endl;

    auto counts = count(SmallGraph("topoGraph.txt"), patternsToMine, nthreads);
    printResults(counts);
  }

  return 0;
}