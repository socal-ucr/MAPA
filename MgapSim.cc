#include <iostream>
#include <string>

#include "Mgap.hh"

int main(int argc, char** argv) {
  std::string jobsFilename;
  size_t totRuntime;
  std::string mgapPolicy;

  if (argc == 1)
  {
    std::cout << argv[0] << " POLICYNAME JOBFILE" << std::endl;
    exit(0);
  }

  if (argc == 2)
  {
    mgapPolicy = "baselineV1";
  }
  else if (argc == 3)
  {
    // Change to baseline as default
    mgapPolicy = argv[1];
  }

  jobsFilename = argv[argc - 1];

  BwMap bwmap = getBwMat("dgx-v");
  SmallGraph topology = cubemesh();
  uint32_t numGpus = getNumGpusPerNode("dgx-v");
  GpuSystem gpuSys = GpuSystem(topology, bwmap, numGpus);

  totRuntime = simulate(jobsFilename, gpuSys, mgapPolicy);
  std::cout << "Total Runtime (cycles) " << totRuntime << std::endl;

  return (0);
}