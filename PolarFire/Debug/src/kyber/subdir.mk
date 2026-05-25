################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/kyber/cbd.c \
../src/kyber/fips202.c \
../src/kyber/indcpa.c \
../src/kyber/kem.c \
../src/kyber/ntt.c \
../src/kyber/poly.c \
../src/kyber/polyvec.c \
../src/kyber/randombytes.c \
../src/kyber/reduce.c \
../src/kyber/symmetric-shake.c \
../src/kyber/verify.c 

OBJS += \
./src/kyber/cbd.o \
./src/kyber/fips202.o \
./src/kyber/indcpa.o \
./src/kyber/kem.o \
./src/kyber/ntt.o \
./src/kyber/poly.o \
./src/kyber/polyvec.o \
./src/kyber/randombytes.o \
./src/kyber/reduce.o \
./src/kyber/symmetric-shake.o \
./src/kyber/verify.o 

C_DEPS += \
./src/kyber/cbd.d \
./src/kyber/fips202.d \
./src/kyber/indcpa.d \
./src/kyber/kem.d \
./src/kyber/ntt.d \
./src/kyber/poly.d \
./src/kyber/polyvec.d \
./src/kyber/randombytes.d \
./src/kyber/reduce.d \
./src/kyber/symmetric-shake.d \
./src/kyber/verify.d 


# Each subdirectory must supply rules for building sources it contributes
src/kyber/%.o: ../src/kyber/%.c src/kyber/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -msmall-data-limit=8 -mno-save-restore -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -Os -fmessage-length=0 -ffunction-sections -fdata-sections  -g3 -DRANDOM_TRNG -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\kyber" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\kernel" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\include\lib" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\startup" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\bsp" -I"E:\Dropbox\Work\1.Professor\Technical\PolarFireSoC\Projects\SoftConsoleWorkspace\ucx_and_pqc_mpfs_u54\src\drivers" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


