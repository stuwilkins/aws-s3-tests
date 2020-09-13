# A minimal CMakeLists.txt file for the AWS SDK for C++.

# The minimum version of CMake that will work.
cmake_minimum_required(VERSION 3.8)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# The project name.
project(s3-demo)

# Locate the AWS SDK for C++ package.
# Requires that you build with:
#   -Daws-sdk-cpp_DIR=/path/to/sdk_build
# or export/set:
#   CMAKE_PREFIX_PATH=/path/to/sdk_build
find_package(AWSSDK REQUIRED COMPONENTS s3 )
find_package(TIFF REQUIRED COMPONENTS CXX)

# Link to the AWS SDK for C++ shared libraries.
add_definitions(-DUSE_IMPORT_EXPORT)

# The executable name and its source files.
add_executable(s3-demo create_bucket.cpp)

include_directories(${TIFF_INCLUDE_DIRS})

# The libraries used by your executable.
target_link_libraries(s3-demo ${TIFF_LIBRARIES} ${TIFFXX_LIBRARIES} ${AWSSDK_LINK_LIBRARIES})