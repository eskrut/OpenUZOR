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
