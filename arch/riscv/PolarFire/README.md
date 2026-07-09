# UCX-OS-ARINC653 on PolarFire Soc Discovery kit

## Make UCX-OS-ARINC653 works on SoftConsole

* Create a workspace
* Create a new C Project named PolarFire
* Import this folder in the project from File System
* Import include folder of ucx-os in the project
* exclude include/arinc/static from build configuration
* Import spfbench into the src folder of the project, exclude **test_applications** folder from build configuration
* Import these files into ucx/arch_common folder:  
    * ieee754.c
    * ieee754.h
    * math.c
    * math.h
    * muldiv.c
* Import these folders into ucx folder:
    * arinc
    * init
    * kernel
    * lib
* exclude src/ucx/arinc/static from build configuration
* Add these flags to compilation configuration: -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -DMEASURE_STATIC=1 -DPERFORMANCE_PRINT -DTEST_PERFXX (flag -DTEST_PERFXX is optionnal, you can use it when you want to run test_perf number XX)

* Include these paths into Build Settings:
    * "${workspace_loc:/${ProjName}/PolarFire/include/arinc}"
    * "${workspace_loc:/${ProjName}/PolarFire/include/testperf}"
    * "${workspace_loc:/${ProjName}/PolarFire/src/spfbench/support_files/include}"
    * "${workspace_loc:/${ProjName}/PolarFire/include}"
    * "${workspace_loc:/${ProjName}/PolarFire/src}"
    * "${workspace_loc:/${ProjName}/PolarFire/src/startup}"
    * "${workspace_loc:/${ProjName}/PolarFire/src/bsp}"
    * "${workspace_loc:/${ProjName}/PolarFire/src/drivers}"
    * "${workspace_loc:/${ProjName}/PolarFire/include/kernel}"
    * "${workspace_loc:/${ProjName}/PolarFire/include/lib}"

* Add the ldscript src/startup/linker.ld

