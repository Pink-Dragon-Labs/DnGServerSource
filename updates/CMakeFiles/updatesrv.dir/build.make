# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/realm/cmake/source

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/realm/cmake/source

# Include any dependencies generated for this target.
include updates/CMakeFiles/updatesrv.dir/depend.make

# Include the progress variables for this target.
include updates/CMakeFiles/updatesrv.dir/progress.make

# Include the compile flags for this target's objects.
include updates/CMakeFiles/updatesrv.dir/flags.make

updates/CMakeFiles/updatesrv.dir/main.cpp.o: updates/CMakeFiles/updatesrv.dir/flags.make
updates/CMakeFiles/updatesrv.dir/main.cpp.o: updates/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/realm/cmake/source/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object updates/CMakeFiles/updatesrv.dir/main.cpp.o"
	cd /home/realm/cmake/source/updates && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/updatesrv.dir/main.cpp.o -c /home/realm/cmake/source/updates/main.cpp

updates/CMakeFiles/updatesrv.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/updatesrv.dir/main.cpp.i"
	cd /home/realm/cmake/source/updates && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/realm/cmake/source/updates/main.cpp > CMakeFiles/updatesrv.dir/main.cpp.i

updates/CMakeFiles/updatesrv.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/updatesrv.dir/main.cpp.s"
	cd /home/realm/cmake/source/updates && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/realm/cmake/source/updates/main.cpp -o CMakeFiles/updatesrv.dir/main.cpp.s

updates/CMakeFiles/updatesrv.dir/main.cpp.o.requires:

.PHONY : updates/CMakeFiles/updatesrv.dir/main.cpp.o.requires

updates/CMakeFiles/updatesrv.dir/main.cpp.o.provides: updates/CMakeFiles/updatesrv.dir/main.cpp.o.requires
	$(MAKE) -f updates/CMakeFiles/updatesrv.dir/build.make updates/CMakeFiles/updatesrv.dir/main.cpp.o.provides.build
.PHONY : updates/CMakeFiles/updatesrv.dir/main.cpp.o.provides

updates/CMakeFiles/updatesrv.dir/main.cpp.o.provides.build: updates/CMakeFiles/updatesrv.dir/main.cpp.o


# Object files for target updatesrv
updatesrv_OBJECTS = \
"CMakeFiles/updatesrv.dir/main.cpp.o"

# External object files for target updatesrv
updatesrv_EXTERNAL_OBJECTS =

bin/updatesrv: updates/CMakeFiles/updatesrv.dir/main.cpp.o
bin/updatesrv: updates/CMakeFiles/updatesrv.dir/build.make
bin/updatesrv: lib/libglobal.a
bin/updatesrv: /usr/lib/i386-linux-gnu/libmysqlclient.so
bin/updatesrv: updates/CMakeFiles/updatesrv.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/realm/cmake/source/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/updatesrv"
	cd /home/realm/cmake/source/updates && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/updatesrv.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
updates/CMakeFiles/updatesrv.dir/build: bin/updatesrv

.PHONY : updates/CMakeFiles/updatesrv.dir/build

updates/CMakeFiles/updatesrv.dir/requires: updates/CMakeFiles/updatesrv.dir/main.cpp.o.requires

.PHONY : updates/CMakeFiles/updatesrv.dir/requires

updates/CMakeFiles/updatesrv.dir/clean:
	cd /home/realm/cmake/source/updates && $(CMAKE_COMMAND) -P CMakeFiles/updatesrv.dir/cmake_clean.cmake
.PHONY : updates/CMakeFiles/updatesrv.dir/clean

updates/CMakeFiles/updatesrv.dir/depend:
	cd /home/realm/cmake/source && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/realm/cmake/source /home/realm/cmake/source/updates /home/realm/cmake/source /home/realm/cmake/source/updates /home/realm/cmake/source/updates/CMakeFiles/updatesrv.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : updates/CMakeFiles/updatesrv.dir/depend
