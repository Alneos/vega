################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Systus/SystusModel.cpp \
../Systus/SystusRunner.cpp \
../Systus/SystusWriter.cpp 

OBJS += \
./Systus/SystusModel.o \
./Systus/SystusRunner.o \
./Systus/SystusWriter.o 

CPP_DEPS += \
./Systus/SystusModel.d \
./Systus/SystusRunner.d \
./Systus/SystusWriter.d 


# Each subdirectory must supply rules for building sources it contributes
Systus/%.o: ../Systus/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


