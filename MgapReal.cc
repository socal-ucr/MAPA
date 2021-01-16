#include <iostream>
#include <string>

#include "MgapReal.hh"

int main(int argc, char **argv)
{
  std::string mgapPolicy;
  std::string systemArch;
  std::string jobsFilename;

  parseArgs(argc, argv, mgapPolicy, systemArch, jobsFilename);

  auto gpuSys = GpuSystem(systemArch);

  auto totRuntime = realRun(jobsFilename, gpuSys, mgapPolicy);
  std::cout << "Total Runtime " << totRuntime << std::endl;

  return (0);
}
