################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ucx/arch_common/ieee754.c \
../src/ucx/arch_common/math.c \
../src/ucx/arch_common/muldiv.c 

OBJS += \
./src/ucx/arch_common/ieee754.o \
./src/ucx/arch_common/math.o \
./src/ucx/arch_common/muldiv.o 

C_DEPS += \
./src/ucx/arch_common/ieee754.d \
./src/ucx/arch_common/math.d \
./src/ucx/arch_common/muldiv.d 


# Each subdirectory must supply rules for building sources it contributes
src/ucx/arch_common/%.o: ../src/ucx/arch_common/%.c src/ucx/arch_common/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -msmall-data-limit=8 -mno-save-restore -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -Os -fmessage-length=0 -ffunction-sections -fdata-sections  -g3 -DRANDOM_TRNG -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\kyber" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\kernel" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\lib" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\startup" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\bsp" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\drivers" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


