################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/SubSystemModules/PowerManagment/EPS.c \
../src/SubSystemModules/PowerManagment/EPSOperationModes.c 

OBJS += \
./src/SubSystemModules/PowerManagment/EPS.o \
./src/SubSystemModules/PowerManagment/EPSOperationModes.o 

C_DEPS += \
./src/SubSystemModules/PowerManagment/EPS.d \
./src/SubSystemModules/PowerManagment/EPSOperationModes.d 


# Each subdirectory must supply rules for building sources it contributes
src/SubSystemModules/PowerManagment/%.o: ../src/SubSystemModules/PowerManagment/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -std=c99 -O0 -fmessage-length=0 -ffunction-sections -Wall -Wextra  -g -std=c99 -Dsdram -Dat91sam9g20 -DTRACE_LEVEL=5 -DDEBUG=1 -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:/Users/97250/Documents/GitHub/GlobusSatProject/GlobusSatProject/..//satellite-subsystems/satellite-subsystems/include" -I"C:\Users\97250\Documents\GitHub\GlobusSatProject\GlobusSatProject\src" -I"C:/Users/97250/Documents/GitHub/GlobusSatProject/GlobusSatProject/..//hal/hcc/include" -I"C:/Users/97250/Documents/GitHub/GlobusSatProject/GlobusSatProject/..//hal/freertos/include" -I"C:/Users/97250/Documents/GitHub/GlobusSatProject/GlobusSatProject/..//hal/hal/include" -I"C:/Users/97250/Documents/GitHub/GlobusSatProject/GlobusSatProject/..//hal/at91/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


