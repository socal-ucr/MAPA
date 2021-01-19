# Multi-GPU Allocation Policy (MGAP) framework

The goal of this project is to study multi-GPU allocation/scheduling policies on different multi-GPU architecture systems.

## Steps to run

0. Install GCC >= 9.3
1. `$ make`.
2. `$ ./generateJobs`.
3. Run Jobs.
   a. Run simulations or real runs for all policies on an architecture.
      `$ ./launch-jobs.sh [--real | --sim] SYSTEM_ARCH JOBS_FILE`
   b. Run individually.
      `$ ./mgapReal POLICY_NAME SYSTEM_ARCH JOBFILE`
      `$ ./mgapSim POLICY_NAME SYSTEM_ARCH JOBFILE`
4. If running real jobs (`mgapReal`), Caffe must be compiled with GCC <= 5.4.
