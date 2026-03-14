cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
project(blindfactory)

set(SDLMIXER_OPUS ON)
set(SDLMIXER_MIDI_NATIVE OFF)
set(SDLMIXER_MIDI OFF)
set(SDLMIXER_MIDI_TIMIDITY OFF)
set(SDLMIXER_AU OFF)
set(SDLMIXER_GME OFF)
set(SDLMIXER_VOC OFF)
set(SDLMIXER_WAVPACK OFF)
set(SDLMIXER_MOD OFF)
set(SDLMIXER_MOD_XMP OFF)
set(SDLMIXER_MP3_DRMP3 OFF)
set(SDLMIXER_MP3_MPG123 OFF)
set(SDLMIXER_VORBIS_VORBISFILE OFF)
set(SDLMIXER_VORBIS_STB OFF)
set(SDLMIXER_WAVE ON)
set(SDLMIXER_FLAC_DRFLAC OFF)
set(SDLMIXER_FLAC_LIBFLAC OFF)
set(SDLMIXER_AIFF OFF)
set(SDLMIXER_VENDORED ON)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL3_mixer EXCLUDE_FROM_ALL)

add_executable(blindfactory WIN32 blindfactory.cpp SimplexNoise.cpp SimplexNoise.h)

target_include_directories(blindfactory PRIVATE vendored/include)

# Link to the actual SDL3 library.
target_link_libraries(blindfactory PRIVATE SDL3::SDL3 SDL3_mixer::SDL3_mixer)
