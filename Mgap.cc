#include <iostream>
#include <string>

#include "Mgap.hh"

int main(int argc, char** argv) {
  std::string jobsFilename;
  size_t totRuntime;

  if (argc < 2)
  {
    std::cout << argv[0] << " JOBFILE" << std::endl;
    exit(0);
  }

  jobsFilename = argv[argc - 1];

  BwMap bwmap = getBwMat("dgx-v");
  SmallGraph topology = cubemesh();
  GpuSystem gpuSys = GpuSystem(topology, bwmap);

  totRuntime = simulate(jobsFilename, gpuSys);
  std::cout << "Total Runtime (cycles) " << totRuntime << std::endl;

  return (0);
}