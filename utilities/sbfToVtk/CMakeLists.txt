project(sbfToVtk)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

find_package(sbf REQUIRED)
include_directories(${SBF_INCLUDE_DIRS})

find_package(VTK COMPONENTS vtkCommonCore vtkIOXML vtkFiltersCore REQUIRED NO_MODULE)
#include_directories(${VTK_INCLUDE_DIRS})
include(${VTK_USE_FILE})

find_package(Boost COMPONENTS program_options REQUIRED)

target_link_libraries(${PROJECT_NAME} ${SBF_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
