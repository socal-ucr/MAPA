#include <iostream>
#include <string>

#include "core.h"

int main(int argc, char** argv) {
  std::string jobsFilename;
  size_t totRuntime;
  
  // static std::string mode = "simulate";

  // if (argc > 1)
  // {
  //   if (argv[1] == "--simulate")
  //   {
  //     mode = "simulate";
  //   }
  // }
  // else
  // {
  //   std::cout << "Cannot run real jobs right now." << std::endl;
  //   exit(0);
  // }

  jobsFilename = argv[argc - 1];

  totRuntime = simulate(jobsFilename, dgx_v);
  std::cout << "Total Runtime (cycles) " << totRuntime;

  // if (mode == "simulate")
  // {
  //   totRuntime = simulate(jobsFilename, dgx_v);
  //   std::cout << "Total Runtime (cycles) " << totRuntime;
  // }
  // else
  // {
  //   totRuntime = realRun();
  //   std::cout << "Total Runtime (s) " << totRuntime / 1e6;
  // }

  return (0);
}