# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_SOURCE_DIR = /rabbitmq-c

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /rabbitmq-c

# Include any dependencies generated for this target.
include examples/CMakeFiles/amqp_ssl_connect.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/amqp_ssl_connect.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/amqp_ssl_connect.dir/flags.make

examples/CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.o: examples/CMakeFiles/amqp_ssl_connect.dir/flags.make
examples/CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.o: examples/amqp_ssl_connect.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/rabbitmq-c/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.o"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.o   -c /rabbitmq-c/examples/amqp_ssl_connect.c

examples/CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.i"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /rabbitmq-c/examples/amqp_ssl_connect.c > CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.i

examples/CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.s"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /rabbitmq-c/examples/amqp_ssl_connect.c -o CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.s

examples/CMakeFiles/amqp_ssl_connect.dir/utils.c.o: examples/CMakeFiles/amqp_ssl_connect.dir/flags.make
examples/CMakeFiles/amqp_ssl_connect.dir/utils.c.o: examples/utils.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/rabbitmq-c/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object examples/CMakeFiles/amqp_ssl_connect.dir/utils.c.o"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/amqp_ssl_connect.dir/utils.c.o   -c /rabbitmq-c/examples/utils.c

examples/CMakeFiles/amqp_ssl_connect.dir/utils.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/amqp_ssl_connect.dir/utils.c.i"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /rabbitmq-c/examples/utils.c > CMakeFiles/amqp_ssl_connect.dir/utils.c.i

examples/CMakeFiles/amqp_ssl_connect.dir/utils.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/amqp_ssl_connect.dir/utils.c.s"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /rabbitmq-c/examples/utils.c -o CMakeFiles/amqp_ssl_connect.dir/utils.c.s

examples/CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.o: examples/CMakeFiles/amqp_ssl_connect.dir/flags.make
examples/CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.o: examples/unix/platform_utils.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/rabbitmq-c/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object examples/CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.o"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.o   -c /rabbitmq-c/examples/unix/platform_utils.c

examples/CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.i"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /rabbitmq-c/examples/unix/platform_utils.c > CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.i

examples/CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.s"
	cd /rabbitmq-c/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /rabbitmq-c/examples/unix/platform_utils.c -o CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.s

# Object files for target amqp_ssl_connect
amqp_ssl_connect_OBJECTS = \
"CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.o" \
"CMakeFiles/amqp_ssl_connect.dir/utils.c.o" \
"CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.o"

# External object files for target amqp_ssl_connect
amqp_ssl_connect_EXTERNAL_OBJECTS =

examples/amqp_ssl_connect: examples/CMakeFiles/amqp_ssl_connect.dir/amqp_ssl_connect.c.o
examples/amqp_ssl_connect: examples/CMakeFiles/amqp_ssl_connect.dir/utils.c.o
examples/amqp_ssl_connect: examples/CMakeFiles/amqp_ssl_connect.dir/unix/platform_utils.c.o
examples/amqp_ssl_connect: examples/CMakeFiles/amqp_ssl_connect.dir/build.make
examples/amqp_ssl_connect: librabbitmq/librabbitmq.so.4.3.1
examples/amqp_ssl_connect: /usr/lib/x86_64-linux-gnu/libssl.so
examples/amqp_ssl_connect: /usr/lib/x86_64-linux-gnu/libcrypto.so
examples/amqp_ssl_connect: examples/CMakeFiles/amqp_ssl_connect.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/rabbitmq-c/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable amqp_ssl_connect"
	cd /rabbitmq-c/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/amqp_ssl_connect.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/amqp_ssl_connect.dir/build: examples/amqp_ssl_connect

.PHONY : examples/CMakeFiles/amqp_ssl_connect.dir/build

examples/CMakeFiles/amqp_ssl_connect.dir/clean:
	cd /rabbitmq-c/examples && $(CMAKE_COMMAND) -P CMakeFiles/amqp_ssl_connect.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/amqp_ssl_connect.dir/clean

examples/CMakeFiles/amqp_ssl_connect.dir/depend:
	cd /rabbitmq-c && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /rabbitmq-c /rabbitmq-c/examples /rabbitmq-c /rabbitmq-c/examples /rabbitmq-c/examples/CMakeFiles/amqp_ssl_connect.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/amqp_ssl_connect.dir/depend
