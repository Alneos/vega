################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Test/Nastran/NastranParser_test.cpp \
../Test/Nastran/NastranTokenizer_test.cpp 

OBJS += \
./Test/Nastran/NastranParser_test.o \
./Test/Nastran/NastranTokenizer_test.o 

CPP_DEPS += \
./Test/Nastran/NastranParser_test.d \
./Test/Nastran/NastranTokenizer_test.d 


# Each subdirectory must supply rules for building sources it contributes
Test/Nastran/%.o: ../Test/Nastran/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


