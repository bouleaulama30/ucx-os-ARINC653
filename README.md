# UCX/OS ARINC 653
**ARINC 653** is a widely used avionic standard for critical system. 
This standard ensures spatial and temporal isolation between program, thanks to a robust partition system and a two-level scheduler (one for partitions and one for processes).

This project is the adaptation of UCX/OS to the **ARINC 653** standard.

UCX/OS is a preemptive nanokernel RTOS for microcontrollers, aimed to be easily ported. The kernel implements a lightweight multitasking environment in a single address space (based on tasks and coroutines), using a minimum amount of resources.

Currently, UCX/OS supports the following targets:

#### RISC-V (32 / 64 bit)
- RV32I (Qemu) / RV32IMA (SMP)
- RV64I (Qemu) / RV64IMA (SMP)
- HF-RISCV (RV32E / RV32I)

#### ARM (32 bit)
- Versatilepb (Qemu)
- STM32F401 / STM32F411 (BlackPill, Nucleo)
- STM32F407 (Discovery)

#### MIPS (32 bit)
- HF-RISC

#### AVR (8 bit)
- ATMEGA328p (Arduino Nano)
- ATMEGA2560 (Arduino Mega)
- ATMEGA32


## Supported toolchains

Different toolchains based on GCC and LLVM can be used to build the kernel and applications. If you want to build a cross-compiler from scratch, check the *sjohann81/toolchains* repository for build scripts.


## Building example applications

In order to build the examples, a cross compiler toolchain has to be installed in the build machine. This toolchain will be used to assemble and compile the kernel sources, as well as user application sources and to build a single binary image that can be uploaded to a board or run in a simulator. The build process will generate several files in the *build/target* directory. In order to simplify recurrent application builds, the OS is compiled into a static library (libucxos.a) and application objects are linked with this library in the final build stage.

You can use several **arinc 653** application:
* **arinc_app**: the basic app where you can develop on it
* **arinc_app_demo1**: a basic waker and worker app with a sensor periodic reader
* **arinc_app_demo2**: test the error handler at different level
* **arinc_app_demo3**: a round robin test on two aperiodic processes
* **arinc_app_demo4**: communication interpartition between partition 1 and partition 2

To build an **arinc 653** application on **qemu** by using **riscv-32** architecture type: '*make all ARINC_APP_TARGET=arinc_application_name*'

This target will:
* Remove previous OS files
* Copy the right *static configuration* into the current static configuration file
* Compile the OS to the target *riscv32-qemu*
* Link the application to the OS
* Launch the application with the duration *DURATION* specified in Makefile variable (by default 1s)
* Save the logs into the *./debug/test.txt* file

## Debug an application 
To debug the current application compile with the os, you can run in one terminal '*make qemu_debug*' and in other one type '*make gdb*' or '*make multiarch-gdb*' depending on your computer. You can add a '*-g*' flag in *CFLAGS* of the Makefile to add debug symbole.