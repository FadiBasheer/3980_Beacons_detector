# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

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
CMAKE_COMMAND = /home/fadi/Downloads/CLion-2021.2.1/clion-2021.2.1/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/fadi/Downloads/CLion-2021.2.1/clion-2021.2.1/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/fadi/work/Project1_3980

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/fadi/work/Project1_3980/cmake-build-debug

# Utility rule file for docs.

# Include any custom commands dependencies for this target.
include docs/CMakeFiles/docs.dir/compiler_depend.make

# Include the progress variables for this target.
include docs/CMakeFiles/docs.dir/progress.make

docs/CMakeFiles/docs: docs/Doxyfile.docs
docs/CMakeFiles/docs: ../docs/mainpage.md
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/fadi/work/Project1_3980/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generate API documentation for docs"
	cd /home/fadi/work/Project1_3980/docs && /home/fadi/Downloads/CLion-2021.2.1/clion-2021.2.1/bin/cmake/linux/bin/cmake -E make_directory /home/fadi/work/Project1_3980/cmake-build-debug/docs
	cd /home/fadi/work/Project1_3980/docs && /usr/bin/doxygen /home/fadi/work/Project1_3980/cmake-build-debug/docs/Doxyfile.docs

docs: docs/CMakeFiles/docs
docs: docs/CMakeFiles/docs.dir/build.make
.PHONY : docs

# Rule to build all files generated by this target.
docs/CMakeFiles/docs.dir/build: docs
.PHONY : docs/CMakeFiles/docs.dir/build

docs/CMakeFiles/docs.dir/clean:
	cd /home/fadi/work/Project1_3980/cmake-build-debug/docs && $(CMAKE_COMMAND) -P CMakeFiles/docs.dir/cmake_clean.cmake
.PHONY : docs/CMakeFiles/docs.dir/clean

docs/CMakeFiles/docs.dir/depend:
	cd /home/fadi/work/Project1_3980/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/fadi/work/Project1_3980 /home/fadi/work/Project1_3980/docs /home/fadi/work/Project1_3980/cmake-build-debug /home/fadi/work/Project1_3980/cmake-build-debug/docs /home/fadi/work/Project1_3980/cmake-build-debug/docs/CMakeFiles/docs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : docs/CMakeFiles/docs.dir/depend

