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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.5.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.5.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/stefan/Projects/MIP/VolViz/build/glfw

# Include any dependencies generated for this target.
include tests/CMakeFiles/gamma.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/gamma.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/gamma.dir/flags.make

tests/CMakeFiles/gamma.dir/gamma.c.o: tests/CMakeFiles/gamma.dir/flags.make
tests/CMakeFiles/gamma.dir/gamma.c.o: /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/tests/gamma.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/stefan/Projects/MIP/VolViz/build/glfw/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/gamma.dir/gamma.c.o"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/gamma.dir/gamma.c.o   -c /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/tests/gamma.c

tests/CMakeFiles/gamma.dir/gamma.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gamma.dir/gamma.c.i"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/tests/gamma.c > CMakeFiles/gamma.dir/gamma.c.i

tests/CMakeFiles/gamma.dir/gamma.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gamma.dir/gamma.c.s"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/tests/gamma.c -o CMakeFiles/gamma.dir/gamma.c.s

tests/CMakeFiles/gamma.dir/gamma.c.o.requires:

.PHONY : tests/CMakeFiles/gamma.dir/gamma.c.o.requires

tests/CMakeFiles/gamma.dir/gamma.c.o.provides: tests/CMakeFiles/gamma.dir/gamma.c.o.requires
	$(MAKE) -f tests/CMakeFiles/gamma.dir/build.make tests/CMakeFiles/gamma.dir/gamma.c.o.provides.build
.PHONY : tests/CMakeFiles/gamma.dir/gamma.c.o.provides

tests/CMakeFiles/gamma.dir/gamma.c.o.provides.build: tests/CMakeFiles/gamma.dir/gamma.c.o


tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o: tests/CMakeFiles/gamma.dir/flags.make
tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o: /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/deps/getopt.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/stefan/Projects/MIP/VolViz/build/glfw/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/gamma.dir/__/deps/getopt.c.o   -c /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/deps/getopt.c

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gamma.dir/__/deps/getopt.c.i"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/deps/getopt.c > CMakeFiles/gamma.dir/__/deps/getopt.c.i

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gamma.dir/__/deps/getopt.c.s"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/deps/getopt.c -o CMakeFiles/gamma.dir/__/deps/getopt.c.s

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.requires:

.PHONY : tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.requires

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.provides: tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.requires
	$(MAKE) -f tests/CMakeFiles/gamma.dir/build.make tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.provides.build
.PHONY : tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.provides

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.provides.build: tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o


tests/CMakeFiles/gamma.dir/__/deps/glad.c.o: tests/CMakeFiles/gamma.dir/flags.make
tests/CMakeFiles/gamma.dir/__/deps/glad.c.o: /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/deps/glad.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/stefan/Projects/MIP/VolViz/build/glfw/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object tests/CMakeFiles/gamma.dir/__/deps/glad.c.o"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/gamma.dir/__/deps/glad.c.o   -c /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/deps/glad.c

tests/CMakeFiles/gamma.dir/__/deps/glad.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gamma.dir/__/deps/glad.c.i"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/deps/glad.c > CMakeFiles/gamma.dir/__/deps/glad.c.i

tests/CMakeFiles/gamma.dir/__/deps/glad.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gamma.dir/__/deps/glad.c.s"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/deps/glad.c -o CMakeFiles/gamma.dir/__/deps/glad.c.s

tests/CMakeFiles/gamma.dir/__/deps/glad.c.o.requires:

.PHONY : tests/CMakeFiles/gamma.dir/__/deps/glad.c.o.requires

tests/CMakeFiles/gamma.dir/__/deps/glad.c.o.provides: tests/CMakeFiles/gamma.dir/__/deps/glad.c.o.requires
	$(MAKE) -f tests/CMakeFiles/gamma.dir/build.make tests/CMakeFiles/gamma.dir/__/deps/glad.c.o.provides.build
.PHONY : tests/CMakeFiles/gamma.dir/__/deps/glad.c.o.provides

tests/CMakeFiles/gamma.dir/__/deps/glad.c.o.provides.build: tests/CMakeFiles/gamma.dir/__/deps/glad.c.o


# Object files for target gamma
gamma_OBJECTS = \
"CMakeFiles/gamma.dir/gamma.c.o" \
"CMakeFiles/gamma.dir/__/deps/getopt.c.o" \
"CMakeFiles/gamma.dir/__/deps/glad.c.o"

# External object files for target gamma
gamma_EXTERNAL_OBJECTS =

tests/gamma: tests/CMakeFiles/gamma.dir/gamma.c.o
tests/gamma: tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o
tests/gamma: tests/CMakeFiles/gamma.dir/__/deps/glad.c.o
tests/gamma: tests/CMakeFiles/gamma.dir/build.make
tests/gamma: src/libglfw3.a
tests/gamma: tests/CMakeFiles/gamma.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/stefan/Projects/MIP/VolViz/build/glfw/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable gamma"
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gamma.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/gamma.dir/build: tests/gamma

.PHONY : tests/CMakeFiles/gamma.dir/build

tests/CMakeFiles/gamma.dir/requires: tests/CMakeFiles/gamma.dir/gamma.c.o.requires
tests/CMakeFiles/gamma.dir/requires: tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.requires
tests/CMakeFiles/gamma.dir/requires: tests/CMakeFiles/gamma.dir/__/deps/glad.c.o.requires

.PHONY : tests/CMakeFiles/gamma.dir/requires

tests/CMakeFiles/gamma.dir/clean:
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw/tests && $(CMAKE_COMMAND) -P CMakeFiles/gamma.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/gamma.dir/clean

tests/CMakeFiles/gamma.dir/depend:
	cd /Users/stefan/Projects/MIP/VolViz/build/glfw && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw /Users/stefan/Projects/MIP/VolViz/Dependencies/glfw/tests /Users/stefan/Projects/MIP/VolViz/build/glfw /Users/stefan/Projects/MIP/VolViz/build/glfw/tests /Users/stefan/Projects/MIP/VolViz/build/glfw/tests/CMakeFiles/gamma.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/gamma.dir/depend

