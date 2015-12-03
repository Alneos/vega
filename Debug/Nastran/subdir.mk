################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Nastran/NastranFacade.cpp \
../Nastran/NastranParser.cpp \
../Nastran/NastranParser_geometry.cpp \
../Nastran/NastranRunner.cpp \
../Nastran/NastranTokenizer.cpp \
../Nastran/NastranWriter.cpp 

OBJS += \
./Nastran/NastranFacade.o \
./Nastran/NastranParser.o \
./Nastran/NastranParser_geometry.o \
./Nastran/NastranRunner.o \
./Nastran/NastranTokenizer.o \
./Nastran/NastranWriter.o 

CPP_DEPS += \
./Nastran/NastranFacade.d \
./Nastran/NastranParser.d \
./Nastran/NastranParser_geometry.d \
./Nastran/NastranRunner.d \
./Nastran/NastranTokenizer.d \
./Nastran/NastranWriter.d 


# Each subdirectory must supply rules for building sources it contributes
Nastran/%.o: ../Nastran/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


