project(qSbfToVTK)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
include_directories(../sbfToVtk/src)

if(UNIX)
    list(APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

find_package(sbf REQUIRED)
include_directories(${SBF_INCLUDE_DIRS})

find_package(VTK 6.1.0 REQUIRED)
include_directories(${VTK_INCLUDE_DIRS})

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDE_DIRS})
QT4_WRAP_UI(UISrcs sbftovtkdialog.ui)
QT4_WRAP_CPP(MOCSrcs sbftovtkdialog.h)

add_executable(${PROJECT_NAME} ${SRC_LIST} ../sbfToVtk/src/sbfToVTK.cpp ${MOCSrcs} ${UISrcs})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${SBF_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
