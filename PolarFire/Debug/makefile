################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/ucx/lib/subdir.mk
-include src/ucx/kernel/subdir.mk
-include src/ucx/init/subdir.mk
-include src/ucx/arch_common/subdir.mk
-include src/startup/subdir.mk
-include src/kyber/subdir.mk
-include src/drivers/subdir.mk
-include src/bsp/subdir.mk
-include src/app/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ucx_and_pqc_mpfs_u54
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
ucx_and_pqc_mpfs_u54.hex \

SECONDARY_SIZE += \
ucx_and_pqc_mpfs_u54.siz \


# All Target
all: main-build

# Main-build Target
main-build: ucx_and_pqc_mpfs_u54.elf secondary-outputs

# Tool invocations
ucx_and_pqc_mpfs_u54.elf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU RISC-V Cross C Linker'
	riscv64-unknown-elf-gcc -msmall-data-limit=8 -mno-save-restore -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -Os -fmessage-length=0 -ffunction-sections -fdata-sections  -g3 -T E:/Dropbox/Work/1.Professor/Technical/PolarFireSoC/Projects/SoftConsoleWorkspace/ucx_and_pqc_mpfs_u54/src/startup/linker.ld -Xlinker --gc-sections -Wl,-Map,"ucx_and_pqc_mpfs_u54.map" -nostartfiles -o "ucx_and_pqc_mpfs_u54.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

ucx_and_pqc_mpfs_u54.hex: ucx_and_pqc_mpfs_u54.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU RISC-V Cross Create Flash Image'
	riscv64-unknown-elf-objcopy -O ihex "ucx_and_pqc_mpfs_u54.elf"  "ucx_and_pqc_mpfs_u54.hex"
	@echo 'Finished building: $@'
	@echo ' '

ucx_and_pqc_mpfs_u54.siz: ucx_and_pqc_mpfs_u54.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU RISC-V Cross Print Size'
	riscv64-unknown-elf-size --format=berkeley "ucx_and_pqc_mpfs_u54.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) ucx_and_pqc_mpfs_u54.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build

-include ../makefile.targets
