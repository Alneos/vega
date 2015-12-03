################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Commandline/VegaCommandLine.cpp \
../Commandline/vegamain.cpp 

OBJS += \
./Commandline/VegaCommandLine.o \
./Commandline/vegamain.o 

CPP_DEPS += \
./Commandline/VegaCommandLine.d \
./Commandline/vegamain.d 


# Each subdirectory must supply rules for building sources it contributes
Commandline/%.o: ../Commandline/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


