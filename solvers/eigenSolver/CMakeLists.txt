project(eigenSolver)
set (TEST test_${PROJECT_NAME})
cmake_minimum_required(VERSION 2.8)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

set( SRC_LIST
main.cpp
eigenSolver.cpp
)
set( TESTS_SOURCES
main_tests.cpp
test01.cpp
eigenSolver.cpp
)

add_executable(${PROJECT_NAME} ${SRC_LIST})
add_executable(${TEST} ${TESTS_SOURCES})

find_package(sbf REQUIRED)
include_directories(${SBF_INCLUDE_DIRS})

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Eigen3 REQUIRED NO_MODULE)
include_directories(${Eigen3_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${SBF_LIBRARIES} Eigen3::Eigen)

target_link_libraries(${TEST} ${Boost_LIBRARIES} ${SBF_LIBRARIES} Eigen3::Eigen)
enable_testing()
add_test(${TEST} ${TEST})
