project(ConjGradMethWithPrec)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

include_directories(../../../../libsbf/src)
list(APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
find_library(SBF_LIBRARY sbf)

message("sbf: " ${SBF_LIBRARY})
