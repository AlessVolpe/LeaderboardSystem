# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_SOURCE_DIR = /home/melk0r_/LeaderboardSystem

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/melk0r_/LeaderboardSystem/build

# Include any dependencies generated for this target.
include CMakeFiles/leaderboardsystem.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/leaderboardsystem.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/leaderboardsystem.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/leaderboardsystem.dir/flags.make

CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o: CMakeFiles/leaderboardsystem.dir/flags.make
CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o: ../src/hashmap.c
CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o: CMakeFiles/leaderboardsystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/melk0r_/LeaderboardSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o -MF CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o.d -o CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o -c /home/melk0r_/LeaderboardSystem/src/hashmap.c

CMakeFiles/leaderboardsystem.dir/src/hashmap.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/leaderboardsystem.dir/src/hashmap.c.i"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/melk0r_/LeaderboardSystem/src/hashmap.c > CMakeFiles/leaderboardsystem.dir/src/hashmap.c.i

CMakeFiles/leaderboardsystem.dir/src/hashmap.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/leaderboardsystem.dir/src/hashmap.c.s"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/melk0r_/LeaderboardSystem/src/hashmap.c -o CMakeFiles/leaderboardsystem.dir/src/hashmap.c.s

CMakeFiles/leaderboardsystem.dir/main.c.o: CMakeFiles/leaderboardsystem.dir/flags.make
CMakeFiles/leaderboardsystem.dir/main.c.o: ../main.c
CMakeFiles/leaderboardsystem.dir/main.c.o: CMakeFiles/leaderboardsystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/melk0r_/LeaderboardSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/leaderboardsystem.dir/main.c.o"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/leaderboardsystem.dir/main.c.o -MF CMakeFiles/leaderboardsystem.dir/main.c.o.d -o CMakeFiles/leaderboardsystem.dir/main.c.o -c /home/melk0r_/LeaderboardSystem/main.c

CMakeFiles/leaderboardsystem.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/leaderboardsystem.dir/main.c.i"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/melk0r_/LeaderboardSystem/main.c > CMakeFiles/leaderboardsystem.dir/main.c.i

CMakeFiles/leaderboardsystem.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/leaderboardsystem.dir/main.c.s"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/melk0r_/LeaderboardSystem/main.c -o CMakeFiles/leaderboardsystem.dir/main.c.s

CMakeFiles/leaderboardsystem.dir/src/bostree.c.o: CMakeFiles/leaderboardsystem.dir/flags.make
CMakeFiles/leaderboardsystem.dir/src/bostree.c.o: ../src/bostree.c
CMakeFiles/leaderboardsystem.dir/src/bostree.c.o: CMakeFiles/leaderboardsystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/melk0r_/LeaderboardSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/leaderboardsystem.dir/src/bostree.c.o"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/leaderboardsystem.dir/src/bostree.c.o -MF CMakeFiles/leaderboardsystem.dir/src/bostree.c.o.d -o CMakeFiles/leaderboardsystem.dir/src/bostree.c.o -c /home/melk0r_/LeaderboardSystem/src/bostree.c

CMakeFiles/leaderboardsystem.dir/src/bostree.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/leaderboardsystem.dir/src/bostree.c.i"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/melk0r_/LeaderboardSystem/src/bostree.c > CMakeFiles/leaderboardsystem.dir/src/bostree.c.i

CMakeFiles/leaderboardsystem.dir/src/bostree.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/leaderboardsystem.dir/src/bostree.c.s"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/melk0r_/LeaderboardSystem/src/bostree.c -o CMakeFiles/leaderboardsystem.dir/src/bostree.c.s

CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o: CMakeFiles/leaderboardsystem.dir/flags.make
CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o: ../src/mtwister.c
CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o: CMakeFiles/leaderboardsystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/melk0r_/LeaderboardSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o -MF CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o.d -o CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o -c /home/melk0r_/LeaderboardSystem/src/mtwister.c

CMakeFiles/leaderboardsystem.dir/src/mtwister.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/leaderboardsystem.dir/src/mtwister.c.i"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/melk0r_/LeaderboardSystem/src/mtwister.c > CMakeFiles/leaderboardsystem.dir/src/mtwister.c.i

CMakeFiles/leaderboardsystem.dir/src/mtwister.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/leaderboardsystem.dir/src/mtwister.c.s"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/melk0r_/LeaderboardSystem/src/mtwister.c -o CMakeFiles/leaderboardsystem.dir/src/mtwister.c.s

CMakeFiles/leaderboardsystem.dir/src/player.c.o: CMakeFiles/leaderboardsystem.dir/flags.make
CMakeFiles/leaderboardsystem.dir/src/player.c.o: ../src/player.c
CMakeFiles/leaderboardsystem.dir/src/player.c.o: CMakeFiles/leaderboardsystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/melk0r_/LeaderboardSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/leaderboardsystem.dir/src/player.c.o"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/leaderboardsystem.dir/src/player.c.o -MF CMakeFiles/leaderboardsystem.dir/src/player.c.o.d -o CMakeFiles/leaderboardsystem.dir/src/player.c.o -c /home/melk0r_/LeaderboardSystem/src/player.c

CMakeFiles/leaderboardsystem.dir/src/player.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/leaderboardsystem.dir/src/player.c.i"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/melk0r_/LeaderboardSystem/src/player.c > CMakeFiles/leaderboardsystem.dir/src/player.c.i

CMakeFiles/leaderboardsystem.dir/src/player.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/leaderboardsystem.dir/src/player.c.s"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/melk0r_/LeaderboardSystem/src/player.c -o CMakeFiles/leaderboardsystem.dir/src/player.c.s

CMakeFiles/leaderboardsystem.dir/src/commons.c.o: CMakeFiles/leaderboardsystem.dir/flags.make
CMakeFiles/leaderboardsystem.dir/src/commons.c.o: ../src/commons.c
CMakeFiles/leaderboardsystem.dir/src/commons.c.o: CMakeFiles/leaderboardsystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/melk0r_/LeaderboardSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/leaderboardsystem.dir/src/commons.c.o"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/leaderboardsystem.dir/src/commons.c.o -MF CMakeFiles/leaderboardsystem.dir/src/commons.c.o.d -o CMakeFiles/leaderboardsystem.dir/src/commons.c.o -c /home/melk0r_/LeaderboardSystem/src/commons.c

CMakeFiles/leaderboardsystem.dir/src/commons.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/leaderboardsystem.dir/src/commons.c.i"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/melk0r_/LeaderboardSystem/src/commons.c > CMakeFiles/leaderboardsystem.dir/src/commons.c.i

CMakeFiles/leaderboardsystem.dir/src/commons.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/leaderboardsystem.dir/src/commons.c.s"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/melk0r_/LeaderboardSystem/src/commons.c -o CMakeFiles/leaderboardsystem.dir/src/commons.c.s

# Object files for target leaderboardsystem
leaderboardsystem_OBJECTS = \
"CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o" \
"CMakeFiles/leaderboardsystem.dir/main.c.o" \
"CMakeFiles/leaderboardsystem.dir/src/bostree.c.o" \
"CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o" \
"CMakeFiles/leaderboardsystem.dir/src/player.c.o" \
"CMakeFiles/leaderboardsystem.dir/src/commons.c.o"

# External object files for target leaderboardsystem
leaderboardsystem_EXTERNAL_OBJECTS =

leaderboardsystem: CMakeFiles/leaderboardsystem.dir/src/hashmap.c.o
leaderboardsystem: CMakeFiles/leaderboardsystem.dir/main.c.o
leaderboardsystem: CMakeFiles/leaderboardsystem.dir/src/bostree.c.o
leaderboardsystem: CMakeFiles/leaderboardsystem.dir/src/mtwister.c.o
leaderboardsystem: CMakeFiles/leaderboardsystem.dir/src/player.c.o
leaderboardsystem: CMakeFiles/leaderboardsystem.dir/src/commons.c.o
leaderboardsystem: CMakeFiles/leaderboardsystem.dir/build.make
leaderboardsystem: CMakeFiles/leaderboardsystem.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/melk0r_/LeaderboardSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking C executable leaderboardsystem"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/leaderboardsystem.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/leaderboardsystem.dir/build: leaderboardsystem
.PHONY : CMakeFiles/leaderboardsystem.dir/build

CMakeFiles/leaderboardsystem.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/leaderboardsystem.dir/cmake_clean.cmake
.PHONY : CMakeFiles/leaderboardsystem.dir/clean

CMakeFiles/leaderboardsystem.dir/depend:
	cd /home/melk0r_/LeaderboardSystem/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/melk0r_/LeaderboardSystem /home/melk0r_/LeaderboardSystem /home/melk0r_/LeaderboardSystem/build /home/melk0r_/LeaderboardSystem/build /home/melk0r_/LeaderboardSystem/build/CMakeFiles/leaderboardsystem.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/leaderboardsystem.dir/depend

