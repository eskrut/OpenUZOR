project(OpenUZOR)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

cmake_policy(SET CMP0053 NEW)

add_subdirectory(utilities)
add_subdirectory(solvers)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

find_package(Threads REQUIRED)
find_package(sbf REQUIRED)
include_directories(${SBF_INCLUDE_DIRS})

option(MAKE_LIBRARY "Build OpenUZOR components into library" OFF)
if(${MAKE_LIBRARY})
	add_library(${PROJECT_NAME} SHARED ${lib_sources} ${lib_headers})
	target_link_libraries(${PROJECT_NAME} ${lib_depends})
endif()
