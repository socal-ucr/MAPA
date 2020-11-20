#include <iostream>
#include <string>

#include "core.h"

int main(int argc, char** argv) {
  string jobsFilename;
  size_t totRuntime;
  
  static mode = "simulate";

  if (argc > 1)
  {
    if (argv[1] == "--simulate")
    {
      mode = "simulate";
    }
  }
  else
  {
    std::cout << "Cannot run real jobs right now." << std::endl;
    exit(0);
  }

  jobsFilename = argv[argc - 1];
  
  if (mode == "simulate")
  {
    totRuntime = simulate(jobsFilename, hwtopo:dgx_v);
    std::cout << "Total Runtime (cycles) " << totRuntime;
  }
  else
  {
    totRuntime = run(jobsFilename, hwtopo:dgx_v);
    std::cout << "Total Runtime (s) " << totRuntime / 1e6;
  }

  return (0);
}