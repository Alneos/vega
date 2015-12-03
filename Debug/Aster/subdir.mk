################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Aster/AsterFacade.cpp \
../Aster/AsterModel.cpp \
../Aster/AsterRunner.cpp \
../Aster/AsterWriter.cpp 

OBJS += \
./Aster/AsterFacade.o \
./Aster/AsterModel.o \
./Aster/AsterRunner.o \
./Aster/AsterWriter.o 

CPP_DEPS += \
./Aster/AsterFacade.d \
./Aster/AsterModel.d \
./Aster/AsterRunner.d \
./Aster/AsterWriter.d 


# Each subdirectory must supply rules for building sources it contributes
Aster/%.o: ../Aster/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


