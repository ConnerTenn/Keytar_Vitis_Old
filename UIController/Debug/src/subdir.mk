################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/Interrupts.c \
../src/main.c 

OBJS += \
./src/Interrupts.o \
./src/main.o 

C_DEPS += \
./src/Interrupts.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -Wall -O0 -g3 -I/NetDrive/Personal/Projects/Keytar/VitisWorkspace/ZTurnPlatform/export/ZTurnPlatform/sw/ZTurnPlatform/ps7_cortexa9_1/bspinclude/include -I"/NetDrive/Personal/Projects/Keytar/VitisWorkspace/UIController/src/USB" -I"/NetDrive/Personal/Projects/Keytar/VitisWorkspace/UIController/src/Video" -I"/NetDrive/Personal/Projects/Keytar/VitisWorkspace/UIController/src/Video/lvgl" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I/NetDrive/Personal/Projects/Keytar/VitisWorkspace/ZTurnPlatform/export/ZTurnPlatform/sw/ZTurnPlatform/ps7_cortexa9_1/bspinclude/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


