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
include mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/compiler_depend.make

# Include the progress variables for this target.
include mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/progress.make

# Include the compile flags for this target's objects.
include mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/flags.make

mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o: mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/flags.make
mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o: mattanTests/googletest-src/googlemock/src/gmock-all.cc
mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o: mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o"
	cd /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build/googlemock && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o -MF CMakeFiles/gmock.dir/src/gmock-all.cc.o.d -o CMakeFiles/gmock.dir/src/gmock-all.cc.o -c /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-src/googlemock/src/gmock-all.cc

mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gmock.dir/src/gmock-all.cc.i"
	cd /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build/googlemock && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-src/googlemock/src/gmock-all.cc > CMakeFiles/gmock.dir/src/gmock-all.cc.i

mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gmock.dir/src/gmock-all.cc.s"
	cd /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build/googlemock && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-src/googlemock/src/gmock-all.cc -o CMakeFiles/gmock.dir/src/gmock-all.cc.s

# Object files for target gmock
gmock_OBJECTS = \
"CMakeFiles/gmock.dir/src/gmock-all.cc.o"

# External object files for target gmock
gmock_EXTERNAL_OBJECTS =

lib/libgmock.a: mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o
lib/libgmock.a: mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/build.make
lib/libgmock.a: mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library ../../../lib/libgmock.a"
	cd /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build/googlemock && $(CMAKE_COMMAND) -P CMakeFiles/gmock.dir/cmake_clean_target.cmake
	cd /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build/googlemock && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gmock.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/build: lib/libgmock.a
.PHONY : mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/build

mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/clean:
	cd /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build/googlemock && $(CMAKE_COMMAND) -P CMakeFiles/gmock.dir/cmake_clean.cmake
.PHONY : mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/clean

mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/depend:
	cd /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /cs/usr/ellorw.nir/os-projects/OS_ex3 /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-src/googlemock /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build/googlemock /cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : mattanTests/googletest-build/googlemock/CMakeFiles/gmock.dir/depend

