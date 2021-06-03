# Multi-Accelerator Pattern Allocation (MAPA) framework

The goal of this project is to study multi-GPU allocation/scheduling policies on different multi-GPU architecture systems.

## Cloning the repository

`$ git clone --recursive https://github.com/socal-ucr/MAPA`

## Steps to run

0. Install GCC >= 9.3
1. Source environment variables.
      `$ source set_env.sh`
2. `$ make`.
3. `$ ./generateJobs`.
4. Run Jobs.
   a. Run simulations or real runs for all policies on an architecture.
      `$ ./launch-jobs.sh [--real | --sim] SYSTEM_ARCH JOBS_FILE`
   b. Run individually.
      `$ ./mapaReal POLICY_NAME SYSTEM_ARCH JOBFILE`
      `$ ./mapaSim POLICY_NAME SYSTEM_ARCH JOBFILE`
5. If running real jobs (`mapaReal`), Caffe must be compiled with GCC <= 5.4.

### Supported System Architectures (SYSTEM_ARCH)

Real Runs: DGX-V, DGX-P, Summit

Simulation Runs: DGX-V, DGX-P, Summit, Torus-2d, CubeMesh-16.

### Custom accelerator topologies and allocation policies

If you intend to add your own accelerator topologies in [GpuTopology.hh](https://github.com/socal-ucr/MAPA/blob/master/GpuTopology.hh):

1. Specify the topology connections like shown [here](https://github.com/socal-ucr/MAPA/blob/master/GpuTopology.hh#L56-L94)
2. Specify the weights of each edge as shown [here](https://github.com/socal-ucr/MAPA/blob/master/GpuTopology.hh#L191-L227)

Custom policies can be added to [MapaPolicies.hh](https://github.com/socal-ucr/MAPA/blob/master/MapaPolicies.hh).
