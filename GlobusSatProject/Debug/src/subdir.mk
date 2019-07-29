################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/InitSystem.c \
../src/filesystem.c \
../src/main.c 

OBJS += \
./src/InitSystem.o \
./src/filesystem.o \
./src/main.o 

C_DEPS += \
./src/InitSystem.d \
./src/filesystem.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -std=c99 -O0 -fmessage-length=0 -ffunction-sections -Wall -Wextra  -g -std=c99 -Dsdram -Dat91sam9g20 -DTRACE_LEVEL=5 -DDEBUG=1 -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:/ISIS/workspace/GlobusSatProject/..//satellite-subsystems/satellite-subsystems/include" -I"C:\ISIS\workspace\GlobusSatProject\src" -I"C:/ISIS/workspace/GlobusSatProject/..//hal/hcc/include" -I"C:/ISIS/workspace/GlobusSatProject/..//hal/freertos/include" -I"C:/ISIS/workspace/GlobusSatProject/..//hal/hal/include" -I"C:/ISIS/workspace/GlobusSatProject/..//hal/at91/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


