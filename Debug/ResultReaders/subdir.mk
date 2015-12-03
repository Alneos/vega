################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ResultReaders/CSVResultReader.cpp \
../ResultReaders/F06Parser.cpp \
../ResultReaders/ResultReadersFacade.cpp 

OBJS += \
./ResultReaders/CSVResultReader.o \
./ResultReaders/F06Parser.o \
./ResultReaders/ResultReadersFacade.o 

CPP_DEPS += \
./ResultReaders/CSVResultReader.d \
./ResultReaders/F06Parser.d \
./ResultReaders/ResultReadersFacade.d 


# Each subdirectory must supply rules for building sources it contributes
ResultReaders/%.o: ../ResultReaders/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


