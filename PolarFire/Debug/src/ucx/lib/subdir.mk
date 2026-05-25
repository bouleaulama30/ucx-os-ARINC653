################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ucx/lib/console.c \
../src/ucx/lib/dump.c \
../src/ucx/lib/fixed.c \
../src/ucx/lib/libc.c \
../src/ucx/lib/list.c \
../src/ucx/lib/malloc.c \
../src/ucx/lib/queue.c 

OBJS += \
./src/ucx/lib/console.o \
./src/ucx/lib/dump.o \
./src/ucx/lib/fixed.o \
./src/ucx/lib/libc.o \
./src/ucx/lib/list.o \
./src/ucx/lib/malloc.o \
./src/ucx/lib/queue.o 

C_DEPS += \
./src/ucx/lib/console.d \
./src/ucx/lib/dump.d \
./src/ucx/lib/fixed.d \
./src/ucx/lib/libc.d \
./src/ucx/lib/list.d \
./src/ucx/lib/malloc.d \
./src/ucx/lib/queue.d 


# Each subdirectory must supply rules for building sources it contributes
src/ucx/lib/%.o: ../src/ucx/lib/%.c src/ucx/lib/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -msmall-data-limit=8 -mno-save-restore -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -Os -fmessage-length=0 -ffunction-sections -fdata-sections  -g3 -DRANDOM_TRNG -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\kyber" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\kernel" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\lib" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\startup" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\bsp" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\drivers" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


