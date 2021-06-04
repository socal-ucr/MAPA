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

   a. Generate JobFile.
      `$ ./generateJobs JOBS_FILE`

   b. Run simulations or real runs for all policies on an architecture.
      `$ ./launch-jobs.sh [--real | --sim] SYSTEM_ARCH JOBS_FILE`

   c. Run individually.

      `$ ./MapaReal POLICY_NAME SYSTEM_ARCH JOBFILE`

      `$ ./MapaSim POLICY_NAME SYSTEM_ARCH JOBFILE`

      `POLICY_NAME` can be `baseline`, `topoAware`, `greedy`, or `preserve`.

      `SYSTEM_ARCH` can be `dgx-v`, `dgx-p`, `summit`, `torus-2d`, or `cubeMesh-16`. For more information, refer to [Supported System Architectures](https://github.com/socal-ucr/MAPA#supported-system-architectures-system_arch).

5. To run real jobs (`MapaReal`), Caffe must be compiled with GCC <= 5.4.

### Supported System Architectures (SYSTEM_ARCH)

Real Runs: DGX-V, DGX-P, Summit

Simulation Runs: DGX-V, DGX-P, Summit, Torus-2d, CubeMesh-16.

### Custom accelerator topologies and allocation policies

If you intend to add your own accelerator topologies in [GpuTopology.hh](https://github.com/socal-ucr/MAPA/blob/master/GpuTopology.hh):

1. Specify the topology connections like shown [here](https://github.com/socal-ucr/MAPA/blob/master/GpuTopology.hh#L65-L103)
2. Specify the weights of each edge as shown [here](https://github.com/socal-ucr/MAPA/blob/master/GpuTopology.hh#L185-L238)

Custom policies can be added to [MapaPolicies.hh](https://github.com/socal-ucr/MAPA/blob/master/MapaPolicies.hh).
