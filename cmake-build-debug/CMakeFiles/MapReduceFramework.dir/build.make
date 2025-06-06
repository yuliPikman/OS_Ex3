# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /cs/usr/ellorw.nir/os-projects/OS_ex3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/MapReduceFramework.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/MapReduceFramework.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/MapReduceFramework.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MapReduceFramework.dir/flags.make

CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o: CMakeFiles/MapReduceFramework.dir/flags.make
CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o: /cs/usr/ellorw.nir/os-projects/OS_ex3/MapReduceFramework.cpp
CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o: CMakeFiles/MapReduceFramework.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o -MF CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o.d -o CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o -c /cs/usr/ellorw.nir/os-projects/OS_ex3/MapReduceFramework.cpp

CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cs/usr/ellorw.nir/os-projects/OS_ex3/MapReduceFramework.cpp > CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.i

CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cs/usr/ellorw.nir/os-projects/OS_ex3/MapReduceFramework.cpp -o CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.s

CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o: CMakeFiles/MapReduceFramework.dir/flags.make
CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o: /cs/usr/ellorw.nir/os-projects/OS_ex3/Barrier.cpp
CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o: CMakeFiles/MapReduceFramework.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o -MF CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o.d -o CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o -c /cs/usr/ellorw.nir/os-projects/OS_ex3/Barrier.cpp

CMakeFiles/MapReduceFramework.dir/Barrier.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MapReduceFramework.dir/Barrier.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cs/usr/ellorw.nir/os-projects/OS_ex3/Barrier.cpp > CMakeFiles/MapReduceFramework.dir/Barrier.cpp.i

CMakeFiles/MapReduceFramework.dir/Barrier.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MapReduceFramework.dir/Barrier.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cs/usr/ellorw.nir/os-projects/OS_ex3/Barrier.cpp -o CMakeFiles/MapReduceFramework.dir/Barrier.cpp.s

# Object files for target MapReduceFramework
MapReduceFramework_OBJECTS = \
"CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o" \
"CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o"

# External object files for target MapReduceFramework
MapReduceFramework_EXTERNAL_OBJECTS =

libMapReduceFramework.a: CMakeFiles/MapReduceFramework.dir/MapReduceFramework.cpp.o
libMapReduceFramework.a: CMakeFiles/MapReduceFramework.dir/Barrier.cpp.o
libMapReduceFramework.a: CMakeFiles/MapReduceFramework.dir/build.make
libMapReduceFramework.a: CMakeFiles/MapReduceFramework.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libMapReduceFramework.a"
	$(CMAKE_COMMAND) -P CMakeFiles/MapReduceFramework.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MapReduceFramework.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MapReduceFramework.dir/build: libMapReduceFramework.a
.PHONY : CMakeFiles/MapReduceFramework.dir/build

CMakeFiles/MapReduceFramework.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MapReduceFramework.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MapReduceFramework.dir/clean

CMakeFiles/MapReduceFramework.dir/depend:
	cd /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /cs/usr/ellorw.nir/os-projects/OS_ex3 /cs/usr/ellorw.nir/os-projects/OS_ex3 /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/CMakeFiles/MapReduceFramework.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MapReduceFramework.dir/depend

