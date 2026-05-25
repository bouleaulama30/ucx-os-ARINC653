################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/app/app_main.c 

OBJS += \
./src/app/app_main.o 

C_DEPS += \
./src/app/app_main.d 


# Each subdirectory must supply rules for building sources it contributes
src/app/%.o: ../src/app/%.c src/app/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -msmall-data-limit=8 -mno-save-restore -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -Os -fmessage-length=0 -ffunction-sections -fdata-sections  -g3 -DRANDOM_TRNG -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\kyber" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\kernel" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\lib" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\startup" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\bsp" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\drivers" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


