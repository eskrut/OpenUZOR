project(reorder)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++1y -Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

find_package(sbf REQUIRED)
include_directories(${SBF_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SBF_LIBRARIES})

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

target_link_libraries(${PROJECT_NAME} metis)

file(GLOB sources_loc reorderer.cpp)
file(GLOB headers_loc reorderer.h)

set(lib_sources ${lib_sources} ${sources_loc} PARENT_SCOPE)
set(lib_headers ${lib_headers} ${headers_loc} PARENT_SCOPE)
set(lib_depends ${lib_depends} ${SBF_LIBRARIES} ${Boost_LIBRARIES} metis PARENT_SCOPE)
