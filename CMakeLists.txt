#root CMakeLists.txt

cmake_minimum_required(VERSION 3.0)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)
project(equinox VERSION 10.6.0.0)
message("PROJECT_VERSION = ${PROJECT_VERSION}")
set(PROJECT_VERSION_FULL "${PROJECT_VERSION}-${CMAKE_C_COMPILER_ID}")
message("PROJECT_VERSION_FULL = ${PROJECT_VERSION_FULL}")

#cmake_policy(SET CMP0079 NEW)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

configure_file(
  "${CMAKE_SOURCE_DIR}/config.h.in"
  "${CMAKE_SOURCE_DIR}/src/config.h"
  ) 

if (CMAKE_VERSION VERSION_LESS 3.1)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
else()
  set(CMAKE_C_STANDARD 99)
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -o0 -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -o3 -march=haswell")
add_compile_options(-Wall)

#add_subdirectory(include)
add_subdirectory(src)

list(APPEND EXTRA_LIBS include)
list(APPEND EXTRA_INCLUDES "${CMAKE_SOURCE_DIR}/include")
