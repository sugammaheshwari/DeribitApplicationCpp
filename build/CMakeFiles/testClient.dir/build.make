# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.30.5/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/build

# Include any dependencies generated for this target.
include CMakeFiles/testClient.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/testClient.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/testClient.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testClient.dir/flags.make

CMakeFiles/testClient.dir/src/testClient.cpp.o: CMakeFiles/testClient.dir/flags.make
CMakeFiles/testClient.dir/src/testClient.cpp.o: /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/src/testClient.cpp
CMakeFiles/testClient.dir/src/testClient.cpp.o: CMakeFiles/testClient.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testClient.dir/src/testClient.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testClient.dir/src/testClient.cpp.o -MF CMakeFiles/testClient.dir/src/testClient.cpp.o.d -o CMakeFiles/testClient.dir/src/testClient.cpp.o -c /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/src/testClient.cpp

CMakeFiles/testClient.dir/src/testClient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/testClient.dir/src/testClient.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/src/testClient.cpp > CMakeFiles/testClient.dir/src/testClient.cpp.i

CMakeFiles/testClient.dir/src/testClient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/testClient.dir/src/testClient.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/src/testClient.cpp -o CMakeFiles/testClient.dir/src/testClient.cpp.s

# Object files for target testClient
testClient_OBJECTS = \
"CMakeFiles/testClient.dir/src/testClient.cpp.o"

# External object files for target testClient
testClient_EXTERNAL_OBJECTS =

testClient: CMakeFiles/testClient.dir/src/testClient.cpp.o
testClient: CMakeFiles/testClient.dir/build.make
testClient: /opt/homebrew/lib/libboost_system-mt.dylib
testClient: /opt/homebrew/Cellar/openssl@3/3.3.2/lib/libssl.dylib
testClient: /opt/homebrew/Cellar/openssl@3/3.3.2/lib/libcrypto.dylib
testClient: CMakeFiles/testClient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable testClient"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testClient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testClient.dir/build: testClient
.PHONY : CMakeFiles/testClient.dir/build

CMakeFiles/testClient.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testClient.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testClient.dir/clean

CMakeFiles/testClient.dir/depend:
	cd /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/build /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/build /Users/sugammaheshwari/Projects/Deribit_Cpp/Deribit_OrderManager_OrderBook/build/CMakeFiles/testClient.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/testClient.dir/depend
