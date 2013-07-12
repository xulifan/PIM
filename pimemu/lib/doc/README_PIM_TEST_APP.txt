PIM_TEST application is a part of the PIM SW suite.

The purpose of the PIM_TEST to run a predictable as well as random workload to verify and to stress PIM development environment.

 

Tests are built hierarchically. At the bottom there are 2 kernels taken from the miniFE proxy app.

There are 3 simple test sequences invoking the above kernels.

hc0 is a simple test that runs the kernels on CPU cores of the PIM host.

hg0 is a simple test that runs the kernels on GPU of the PIM host.

hpg0 is a host-spawned PIM gpu test sequence. It runs 2 kernels on a PIM GPU. It runs it on PIM 0 by default.

These 3 sequences usually are launched as threads by more complicate tests.

hpc1 is a host-spawned PIM cpu test. The test spawns CPU threads on every PIM that's presented in the system. Each tread runs the hc0 sequence.

hpg1 is a host-spawned PIM gpu test. The test spawns CPU threads on every PIM that's presented in the system. Each tread runs the hg0 sequence invoking GPU kernels on the proper PIM.

pg1 spawns CPU threads on each PIM in the system. Each tread runs the hg0 sequence invoking GPU kernels on the proper PIM. 

 

PIM_TEST application can run all 6 different tests in any permutations.

 

The basic kernels can be called in loops. The loop counter is set by -loops argument. Currently it's a generic parameter, e.g. affecting every kernel call in any test.

The simple test can be called recursively. The depth of recursion is set by -depth argument. Currently it's a generic parameter, e.g. affecting every simple sequence in any test.

 

 

How to run




>pim_test <options> <tests>\n");


Options:

-h               help.

-ocl_file        GPU kernels file.

-ocl_loc         GPU kernels location.

-ocl_opt         GPU kernels compile options.

-sz              problem size.

-loops           loop limit.

-depth           invocation depth limit.

Tests:

-hc0             host cpu simple test.

-hg0             host gpu simple test.

-hpg0            host-spawned simple PIM gpu test.

-hpg1            host-spawned PIM gpu test.

-hpc1            host-spawned PIM cpu test.

-pg1             PIM gpu test.
