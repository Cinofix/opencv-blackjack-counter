# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale/build

# Utility rule file for run.

# Include the progress variables for this target.
include src/CMakeFiles/run.dir/progress.make

src/CMakeFiles/run: src/blackJack
	cd /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale && /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale/build/src/blackJack

run: src/CMakeFiles/run
run: src/CMakeFiles/run.dir/build.make

.PHONY : run

# Rule to build all files generated by this target.
src/CMakeFiles/run.dir/build: run

.PHONY : src/CMakeFiles/run.dir/build

src/CMakeFiles/run.dir/clean:
	cd /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale/build/src && $(CMAKE_COMMAND) -P CMakeFiles/run.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/run.dir/clean

src/CMakeFiles/run.dir/depend:
	cd /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale/src /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale/build /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale/build/src /home/antonio/Unive/Magistrale/ComputerVision/BlackJackCounter/BlackJack_Finale/build/src/CMakeFiles/run.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/run.dir/depend

