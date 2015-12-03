################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Test/ResultReaders/CSVResultReaderTest.cpp \
../Test/ResultReaders/F06Parser_test.cpp 

OBJS += \
./Test/ResultReaders/CSVResultReaderTest.o \
./Test/ResultReaders/F06Parser_test.o 

CPP_DEPS += \
./Test/ResultReaders/CSVResultReaderTest.d \
./Test/ResultReaders/F06Parser_test.d 


# Each subdirectory must supply rules for building sources it contributes
Test/ResultReaders/%.o: ../Test/ResultReaders/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


