# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ec2-user/s3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ec2-user/s3/s3_build

# Include any dependencies generated for this target.
include CMakeFiles/copyObject.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/copyObject.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/copyObject.dir/flags.make

CMakeFiles/copyObject.dir/copyObject.cpp.o: CMakeFiles/copyObject.dir/flags.make
CMakeFiles/copyObject.dir/copyObject.cpp.o: ../copyObject.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ec2-user/s3/s3_build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/copyObject.dir/copyObject.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/copyObject.dir/copyObject.cpp.o -c /home/ec2-user/s3/copyObject.cpp

CMakeFiles/copyObject.dir/copyObject.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/copyObject.dir/copyObject.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ec2-user/s3/copyObject.cpp > CMakeFiles/copyObject.dir/copyObject.cpp.i

CMakeFiles/copyObject.dir/copyObject.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/copyObject.dir/copyObject.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ec2-user/s3/copyObject.cpp -o CMakeFiles/copyObject.dir/copyObject.cpp.s

CMakeFiles/copyObject.dir/copyObject.cpp.o.requires:
.PHONY : CMakeFiles/copyObject.dir/copyObject.cpp.o.requires

CMakeFiles/copyObject.dir/copyObject.cpp.o.provides: CMakeFiles/copyObject.dir/copyObject.cpp.o.requires
	$(MAKE) -f CMakeFiles/copyObject.dir/build.make CMakeFiles/copyObject.dir/copyObject.cpp.o.provides.build
.PHONY : CMakeFiles/copyObject.dir/copyObject.cpp.o.provides

CMakeFiles/copyObject.dir/copyObject.cpp.o.provides.build: CMakeFiles/copyObject.dir/copyObject.cpp.o

# Object files for target copyObject
copyObject_OBJECTS = \
"CMakeFiles/copyObject.dir/copyObject.cpp.o"

# External object files for target copyObject
copyObject_EXTERNAL_OBJECTS =

copyObject: CMakeFiles/copyObject.dir/copyObject.cpp.o
copyObject: CMakeFiles/copyObject.dir/build.make
copyObject: /home/ec2-user/sdk_build/aws-cpp-sdk-s3/libaws-cpp-sdk-s3.so
copyObject: /home/ec2-user/sdk_build/aws-cpp-sdk-core/libaws-cpp-sdk-core.so
copyObject: /usr/lib64/libcurl.so
copyObject: /usr/lib64/libssl.so
copyObject: /usr/lib64/libcrypto.so
copyObject: /usr/lib64/libz.so
copyObject: CMakeFiles/copyObject.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable copyObject"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/copyObject.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/copyObject.dir/build: copyObject
.PHONY : CMakeFiles/copyObject.dir/build

CMakeFiles/copyObject.dir/requires: CMakeFiles/copyObject.dir/copyObject.cpp.o.requires
.PHONY : CMakeFiles/copyObject.dir/requires

CMakeFiles/copyObject.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/copyObject.dir/cmake_clean.cmake
.PHONY : CMakeFiles/copyObject.dir/clean

CMakeFiles/copyObject.dir/depend:
	cd /home/ec2-user/s3/s3_build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ec2-user/s3 /home/ec2-user/s3 /home/ec2-user/s3/s3_build /home/ec2-user/s3/s3_build /home/ec2-user/s3/s3_build/CMakeFiles/copyObject.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/copyObject.dir/depend
