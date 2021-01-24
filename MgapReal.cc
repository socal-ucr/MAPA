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

  run(jobsFilename, gpuSys, mgapPolicy);

  return (0);
}
