cmake_minimum_required(VERSION 3.2)
set (CMAKE_CXX_STANDARD 11)
project(s3-examples)

# Locate the aws sdk for c++ package.
find_package(aws-sdk-cpp)

# Link to shared libraries.
add_definitions(-DUSE_IMPORT_EXPORT)

# The executables to build.
add_executable(putObject putObject.cpp)
add_executable(deleteObject deleteObject.cpp)
add_executable(copyObject copyObject.cpp)
add_executable(getObject getObject.cpp)

# Link necessary libraries 
target_link_libraries(putObject aws-cpp-sdk-s3)
target_link_libraries(deleteObject aws-cpp-sdk-s3)
target_link_libraries(copyObject aws-cpp-sdk-s3)
target_link_libraries(getObject aws-cpp-sdk-s3)
