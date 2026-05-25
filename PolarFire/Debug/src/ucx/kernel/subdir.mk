################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ucx/kernel/coroutine.c \
../src/ucx/kernel/ecodes.c \
../src/ucx/kernel/message.c \
../src/ucx/kernel/pipe.c \
../src/ucx/kernel/semaphore.c \
../src/ucx/kernel/spinlock.c \
../src/ucx/kernel/syscall.c \
../src/ucx/kernel/timer.c \
../src/ucx/kernel/ucx.c 

OBJS += \
./src/ucx/kernel/coroutine.o \
./src/ucx/kernel/ecodes.o \
./src/ucx/kernel/message.o \
./src/ucx/kernel/pipe.o \
./src/ucx/kernel/semaphore.o \
./src/ucx/kernel/spinlock.o \
./src/ucx/kernel/syscall.o \
./src/ucx/kernel/timer.o \
./src/ucx/kernel/ucx.o 

C_DEPS += \
./src/ucx/kernel/coroutine.d \
./src/ucx/kernel/ecodes.d \
./src/ucx/kernel/message.d \
./src/ucx/kernel/pipe.d \
./src/ucx/kernel/semaphore.d \
./src/ucx/kernel/spinlock.d \
./src/ucx/kernel/syscall.d \
./src/ucx/kernel/timer.d \
./src/ucx/kernel/ucx.d 


# Each subdirectory must supply rules for building sources it contributes
src/ucx/kernel/%.o: ../src/ucx/kernel/%.c src/ucx/kernel/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -msmall-data-limit=8 -mno-save-restore -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -Os -fmessage-length=0 -ffunction-sections -fdata-sections  -g3 -DRANDOM_TRNG -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\kyber" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\kernel" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\lib" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\startup" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\bsp" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\drivers" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


