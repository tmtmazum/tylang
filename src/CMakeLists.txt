cmake_minimum_required(VERSION 2.6)
project(tylang)

set(tylang_VERSION_MAJOR 0)
set(tylang_VERSION_MINOR 1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../bin)

include_directories(.)
include_directories(../external/cppcoretools)

# tyx
file(GLOB tyx_src ./devconsole/*.cpp)
add_executable(tyx ${tyx_src})

# Tests
add_custom_target(all_tests ALL
	COMMAND python testsystem/run_test.py ../tests)

