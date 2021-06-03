#ifndef MAPA_H
#define MAPA_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"
#include "GpuTopology.hh"
#include "TopoUtils.hh"
#include "MapaPolicies.hh"

void parseArgs(int argc, char** argv, std::string& mapaPolicy, std::string& systemArch, std::string& jobsFilename)
{
  if ((argc < 4) || (argc > 4))
  {
    std::cout << argv[0] << " [POLICY_NAME] [SYSTEM_ARCH] JOBFILE" << std::endl;
    exit(0);
  }

  mapaPolicy = argv[1];
  systemArch = argv[2];
  jobsFilename = argv[argc - 1];
}

Allocation choosePattern(PatternVec &patterns, JobItem job, std::string mapaPolicy)
{
  if (!patterns.empty())
  {
    return policyMap[mapaPolicy](patterns, job);
  }
  else
  {
    return {};
  }
}


#endif
