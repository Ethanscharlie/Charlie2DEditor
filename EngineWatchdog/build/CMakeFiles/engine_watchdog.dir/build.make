# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

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
CMAKE_SOURCE_DIR = /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/build

# Include any dependencies generated for this target.
include CMakeFiles/engine_watchdog.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/engine_watchdog.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/engine_watchdog.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/engine_watchdog.dir/flags.make

CMakeFiles/engine_watchdog.dir/src/main.cpp.o: CMakeFiles/engine_watchdog.dir/flags.make
CMakeFiles/engine_watchdog.dir/src/main.cpp.o: /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/src/main.cpp
CMakeFiles/engine_watchdog.dir/src/main.cpp.o: CMakeFiles/engine_watchdog.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/engine_watchdog.dir/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/engine_watchdog.dir/src/main.cpp.o -MF CMakeFiles/engine_watchdog.dir/src/main.cpp.o.d -o CMakeFiles/engine_watchdog.dir/src/main.cpp.o -c /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/src/main.cpp

CMakeFiles/engine_watchdog.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/engine_watchdog.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/src/main.cpp > CMakeFiles/engine_watchdog.dir/src/main.cpp.i

CMakeFiles/engine_watchdog.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/engine_watchdog.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/src/main.cpp -o CMakeFiles/engine_watchdog.dir/src/main.cpp.s

# Object files for target engine_watchdog
engine_watchdog_OBJECTS = \
"CMakeFiles/engine_watchdog.dir/src/main.cpp.o"

# External object files for target engine_watchdog
engine_watchdog_EXTERNAL_OBJECTS =

engine_watchdog: CMakeFiles/engine_watchdog.dir/src/main.cpp.o
engine_watchdog: CMakeFiles/engine_watchdog.dir/build.make
engine_watchdog: CMakeFiles/engine_watchdog.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable engine_watchdog"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/engine_watchdog.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/engine_watchdog.dir/build: engine_watchdog
.PHONY : CMakeFiles/engine_watchdog.dir/build

CMakeFiles/engine_watchdog.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/engine_watchdog.dir/cmake_clean.cmake
.PHONY : CMakeFiles/engine_watchdog.dir/clean

CMakeFiles/engine_watchdog.dir/depend:
	cd /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/build /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/build /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor/EngineWatchdog/build/CMakeFiles/engine_watchdog.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/engine_watchdog.dir/depend
