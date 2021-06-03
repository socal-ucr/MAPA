#ifndef MGAP_H
#define MGAP_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <boost/lexical_cast.hpp>

#include "Peregrine.hh"
#include "GpuTopology.hh"
#include "TopoUtils.hh"
#include "MgapPolicies.hh"

void parseArgs(int argc, char** argv, std::string& mgapPolicy, std::string& systemArch, std::string& jobsFilename)
{
  if ((argc < 4) || (argc > 4))
  {
    std::cout << argv[0] << " [POLICY_NAME] [SYSTEM_ARCH] JOBFILE" << std::endl;
    exit(0);
  }

  mgapPolicy = argv[1];
  systemArch = argv[2];
  jobsFilename = argv[argc - 1];
}

Allocation choosePattern(PatternVec &patterns, JobItem job, std::string mgapPolicy)
{
  if (!patterns.empty())
  {
    return policyMap[mgapPolicy](patterns, job);
  }
  else
  {
    return {};
  }
}


#endif
