cmake_minimum_required(VERSION 3.16)
project(blindfactory)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")


add_subdirectory(vendored/SDL)

add_executable(blindfactory WIN32 blindfactory.cpp SimplexNoise.cpp SimplexNoise.h)

# Link to the actual SDL3 library.
target_link_libraries(blindfactory PRIVATE SDL3::SDL3)
