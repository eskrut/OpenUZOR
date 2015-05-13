project(staticSESolver)
cmake_minimum_required(VERSION 2.8)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++1y -Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

set( SRC_LIST
main.cpp
)

add_executable(${PROJECT_NAME} ${SRC_LIST})

find_package(sbf REQUIRED)
include_directories(${SBF_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${SBF_LIBRARIES})
