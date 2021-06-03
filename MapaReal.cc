#include <iostream>
#include <string>

#include "MapaReal.hh"

int main(int argc, char **argv)
{
  std::string mapaPolicy;
  std::string systemArch;
  std::string jobsFilename;

  parseArgs(argc, argv, mapaPolicy, systemArch, jobsFilename);

  auto gpuSys = GpuSystem(systemArch);

  run(jobsFilename, gpuSys, mapaPolicy);

  return (0);
}
