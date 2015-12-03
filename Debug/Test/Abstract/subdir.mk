################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Test/Abstract/CoordinateSystem_test.cpp \
../Test/Abstract/Dof_test.cpp \
../Test/Abstract/Element_test.cpp \
../Test/Abstract/Mesh_test.cpp \
../Test/Abstract/Model_test.cpp \
../Test/Abstract/Utility_test.cpp 

OBJS += \
./Test/Abstract/CoordinateSystem_test.o \
./Test/Abstract/Dof_test.o \
./Test/Abstract/Element_test.o \
./Test/Abstract/Mesh_test.o \
./Test/Abstract/Model_test.o \
./Test/Abstract/Utility_test.o 

CPP_DEPS += \
./Test/Abstract/CoordinateSystem_test.d \
./Test/Abstract/Dof_test.d \
./Test/Abstract/Element_test.d \
./Test/Abstract/Mesh_test.d \
./Test/Abstract/Model_test.d \
./Test/Abstract/Utility_test.d 


# Each subdirectory must supply rules for building sources it contributes
Test/Abstract/%.o: ../Test/Abstract/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


