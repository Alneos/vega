################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include build/Test/ResultReaders/CMakeFiles/nastran_f06_tests.dir/subdir.mk
-include build/Test/ResultReaders/CMakeFiles/CSVResultReaderTest.dir/subdir.mk
-include build/Test/Nastran/CMakeFiles/nastran_tokenizer_tests.dir/subdir.mk
-include build/Test/Nastran/CMakeFiles/nastran_tests.dir/subdir.mk
-include build/Test/Commandline/CMakeFiles/command_line_test_utils.dir/subdir.mk
-include build/Test/Commandline/CMakeFiles/Nastran2Systus_test.dir/subdir.mk
-include build/Test/Commandline/CMakeFiles/Nastran2Nastran_test.dir/subdir.mk
-include build/Test/Commandline/CMakeFiles/Nastran2Aster_test.dir/subdir.mk
-include build/Test/Abstract/CMakeFiles/Utility_test.dir/subdir.mk
-include build/Test/Abstract/CMakeFiles/Model_test.dir/subdir.mk
-include build/Test/Abstract/CMakeFiles/Mesh_test.dir/subdir.mk
-include build/Test/Abstract/CMakeFiles/Element_test.dir/subdir.mk
-include build/Test/Abstract/CMakeFiles/Dof_test.dir/subdir.mk
-include build/Test/Abstract/CMakeFiles/CoordinateSystem_test.dir/subdir.mk
-include build/Systus/CMakeFiles/systus.dir/subdir.mk
-include build/ResultReaders/CMakeFiles/resultReaders.dir/subdir.mk
-include build/Nastran/CMakeFiles/nastran.dir/subdir.mk
-include build/Commandline/CMakeFiles/vegapp.dir/subdir.mk
-include build/Commandline/CMakeFiles/commandline.dir/subdir.mk
-include build/CMakeFiles/2.8.12.2/CompilerIdCXX/subdir.mk
-include build/CMakeFiles/2.8.12.2/CompilerIdC/subdir.mk
-include build/Aster/CMakeFiles/aster.dir/subdir.mk
-include build/Abstract/CMakeFiles/abstract.dir/subdir.mk
-include Test/ResultReaders/subdir.mk
-include Test/Nastran/subdir.mk
-include Test/Commandline/subdir.mk
-include Test/Abstract/subdir.mk
-include Systus/subdir.mk
-include ResultReaders/subdir.mk
-include Nastran/subdir.mk
-include Commandline/subdir.mk
-include Aster/subdir.mk
-include Abstract/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: vega

# Tool invocations
vega: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L/opt/boost_1_57_0/lib -L/usr/lib/x86_64-linux-gnu/ -o "vega" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) vega
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
