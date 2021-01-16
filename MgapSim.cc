#include <iostream>
#include <string>

#include "MgapSim.hh"

int main(int argc, char** argv) {
  std::string mgapPolicy;
  std::string systemArch;
  std::string jobsFilename;

  parseArgs(argc, argv, mgapPolicy, systemArch, jobsFilename);

  auto gpuSys = GpuSystem(systemArch);

  auto totRuntime = simulate(jobsFilename, gpuSys, mgapPolicy);
  std::cout << "Total Runtime (cycles) " << totRuntime << std::endl;

  return (0);
}
