#include <iostream>
#include <string>

#include "core.h"

int main(int argc, char** argv) {
  std::string jobsFilename;
  size_t totRuntime;

  jobsFilename = argv[argc - 1];

  BwMap bwmap = getBwMat("dgx-v");
  SmallGraph topology = cubemesh();
  GpuSystem gpuSys = GpuSystem(topology, bwmap);

  totRuntime = simulate(jobsFilename, gpuSys);
  std::cout << "Total Runtime (cycles) " << totRuntime;

  return (0);
}