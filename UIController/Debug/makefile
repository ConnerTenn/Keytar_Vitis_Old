################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/Video/lvgl/tests/lv_test_widgets/subdir.mk
-include src/Video/lvgl/tests/lv_test_fonts/subdir.mk
-include src/Video/lvgl/tests/lv_test_core/subdir.mk
-include src/Video/lvgl/tests/subdir.mk
-include src/Video/lvgl/src/lv_widgets/subdir.mk
-include src/Video/lvgl/src/lv_themes/subdir.mk
-include src/Video/lvgl/src/lv_misc/subdir.mk
-include src/Video/lvgl/src/lv_hal/subdir.mk
-include src/Video/lvgl/src/lv_gpu/subdir.mk
-include src/Video/lvgl/src/lv_font/subdir.mk
-include src/Video/lvgl/src/lv_draw/subdir.mk
-include src/Video/lvgl/src/lv_core/subdir.mk
-include src/Video/lvgl/examples/porting/subdir.mk
-include src/Video/subdir.mk
-include src/USB/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
UIController.elf.size \


# All Target
all:
	$(MAKE) --no-print-directory pre-build
	$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: UIController.elf secondary-outputs

# Tool invocations
UIController.elf: $(OBJS) ../src/lscript.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM v7 gcc linker'
	arm-none-eabi-gcc -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -Wl,-build-id=none -specs=Xilinx.spec -Wl,-T -Wl,../src/lscript.ld -L/NetDrive/Personal/Projects/Keytar/VitisWorkspace/ZTurnPlatform/export/ZTurnPlatform/sw/ZTurnPlatform/ps7_cortexa9_1/bsplib/lib -o "UIController.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

UIController.elf.size: UIController.elf
	@echo 'Invoking: ARM v7 Print Size'
	arm-none-eabi-size UIController.elf  |tee "UIController.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(S_UPPER_DEPS)$(C_DEPS)$(ELFSIZE) UIController.elf
	-@echo ' '

pre-build:
	-a9-linaro-pre-build-step
	-@echo ' '

secondary-outputs: $(ELFSIZE)

.PHONY: all clean dependents main-build pre-build

-include ../makefile.targets
