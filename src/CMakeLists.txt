cmake_minimum_required(VERSION 2.6)
project(tylang)

set(tylang_VERSION_MAJOR 0)
set(tylang_VERSION_MINOR 1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../bin)

include_directories(.)
include_directories(../external/cppcoretools)

# common
file(GLOB tycommon_src ./common/*.cpp)
file(GLOB tycommon_hdr ./common/*.h)
add_library(tycommon STATIC ${tycommon_src} ${tycommon_hdr}) 

# token
file(GLOB tytoken_src ./token/*.cpp)
file(GLOB tytoken_hdr ./token/*.h)
add_library(tytoken STATIC ${tytoken_src} ${tytoken_hdr}) 

# parse
file(GLOB typarse_src ./parse/*.cpp)
file(GLOB typarse_hdr ./parse/*.h)
add_library(typarse STATIC ${typarse_src} ${typarse_hdr}) 

# parse
file(GLOB tycgen_src ./cgen/*.cpp)
file(GLOB tycgen_hdr ./cgen/*.h)
add_library(tycgen STATIC ${tycgen_src} ${tycgen_hdr}) 


# tyx
file(GLOB tyx_src ./devconsole/*.cpp)
add_executable(tyx ${tyx_src})
target_link_libraries(tyx tycommon typarse tycgen tytoken)

# Tests
add_custom_target(all_tests ALL
	COMMAND python testsystem/run_test.py ../tests)

