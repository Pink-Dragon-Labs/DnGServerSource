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
include tokenize/CMakeFiles/tokenize.dir/depend.make

# Include the progress variables for this target.
include tokenize/CMakeFiles/tokenize.dir/progress.make

# Include the compile flags for this target's objects.
include tokenize/CMakeFiles/tokenize.dir/flags.make

tokenize/CMakeFiles/tokenize.dir/main.cpp.o: tokenize/CMakeFiles/tokenize.dir/flags.make
tokenize/CMakeFiles/tokenize.dir/main.cpp.o: tokenize/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/realm/cmake/source/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tokenize/CMakeFiles/tokenize.dir/main.cpp.o"
	cd /home/realm/cmake/source/tokenize && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tokenize.dir/main.cpp.o -c /home/realm/cmake/source/tokenize/main.cpp

tokenize/CMakeFiles/tokenize.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tokenize.dir/main.cpp.i"
	cd /home/realm/cmake/source/tokenize && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/realm/cmake/source/tokenize/main.cpp > CMakeFiles/tokenize.dir/main.cpp.i

tokenize/CMakeFiles/tokenize.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tokenize.dir/main.cpp.s"
	cd /home/realm/cmake/source/tokenize && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/realm/cmake/source/tokenize/main.cpp -o CMakeFiles/tokenize.dir/main.cpp.s

tokenize/CMakeFiles/tokenize.dir/main.cpp.o.requires:

.PHONY : tokenize/CMakeFiles/tokenize.dir/main.cpp.o.requires

tokenize/CMakeFiles/tokenize.dir/main.cpp.o.provides: tokenize/CMakeFiles/tokenize.dir/main.cpp.o.requires
	$(MAKE) -f tokenize/CMakeFiles/tokenize.dir/build.make tokenize/CMakeFiles/tokenize.dir/main.cpp.o.provides.build
.PHONY : tokenize/CMakeFiles/tokenize.dir/main.cpp.o.provides

tokenize/CMakeFiles/tokenize.dir/main.cpp.o.provides.build: tokenize/CMakeFiles/tokenize.dir/main.cpp.o


# Object files for target tokenize
tokenize_OBJECTS = \
"CMakeFiles/tokenize.dir/main.cpp.o"

# External object files for target tokenize
tokenize_EXTERNAL_OBJECTS =

bin/tokenize: tokenize/CMakeFiles/tokenize.dir/main.cpp.o
bin/tokenize: tokenize/CMakeFiles/tokenize.dir/build.make
bin/tokenize: lib/libglobal.a
bin/tokenize: /usr/lib/i386-linux-gnu/libmysqlclient.so
bin/tokenize: tokenize/CMakeFiles/tokenize.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/realm/cmake/source/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/tokenize"
	cd /home/realm/cmake/source/tokenize && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tokenize.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tokenize/CMakeFiles/tokenize.dir/build: bin/tokenize

.PHONY : tokenize/CMakeFiles/tokenize.dir/build

tokenize/CMakeFiles/tokenize.dir/requires: tokenize/CMakeFiles/tokenize.dir/main.cpp.o.requires

.PHONY : tokenize/CMakeFiles/tokenize.dir/requires

tokenize/CMakeFiles/tokenize.dir/clean:
	cd /home/realm/cmake/source/tokenize && $(CMAKE_COMMAND) -P CMakeFiles/tokenize.dir/cmake_clean.cmake
.PHONY : tokenize/CMakeFiles/tokenize.dir/clean

tokenize/CMakeFiles/tokenize.dir/depend:
	cd /home/realm/cmake/source && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/realm/cmake/source /home/realm/cmake/source/tokenize /home/realm/cmake/source /home/realm/cmake/source/tokenize /home/realm/cmake/source/tokenize/CMakeFiles/tokenize.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tokenize/CMakeFiles/tokenize.dir/depend

