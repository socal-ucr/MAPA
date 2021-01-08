#include <iostream>
#include <string>

#include "MgapReal.hh"

int main(int argc, char **argv)
{
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
    mgapPolicy = argv[1];
  }

  std::string jobsFilename = argv[argc - 1];

  auto gpuSys = GpuSystem("dgx-v");

  auto totRuntime = realRun(jobsFilename, gpuSys, mgapPolicy);
  std::cout << "Total Runtime " << totRuntime << std::endl;

  return (0);
}