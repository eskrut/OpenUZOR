project(qPrepareSE)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
include_directories(../prepareSE)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

find_package(sbf REQUIRED)
include_directories(${SBF_INCLUDE_DIRS})

set(TARGET_QT_VERSION "4" CACHE STRING "Expected Qt version")
#mark_as_advanced(TARGET_QT_VERSION)

set_property(CACHE TARGET_QT_VERSION PROPERTY STRINGS 4 5)

if(NOT (TARGET_QT_VERSION VERSION_EQUAL "4" OR TARGET_QT_VERSION VERSION_EQUAL "5"))
  message(FATAL_ERROR "Expected value for TARGET_QT_VERSION is either '4' or '5'")
endif()

set(FILES_TO_TRANSLATE ru_ru.ts)

if(TARGET_QT_VERSION VERSION_GREATER "4")
  find_package(Qt5Widgets REQUIRED)
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  add_definitions(${Qt5Widgets_DEFINITIONS})

  qt5_wrap_cpp(MOCSrcs qpreparese.h)

  find_package(Qt5LinguistTools)
  qt5_create_translation(QM qpreparese.cpp ru_ru.ts)

  set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})
else()
  find_package(Qt REQUIRED)
  include(${QT_USE_FILE})
  include_directories(${QT_INCLUDE_DIRS})
  qt4_wrap_cpp(MOCSrcs qpreparese.h)
  qt4_add_translation(QM ${FILES_TO_TRANSLATE})
endif()

add_executable(${PROJECT_NAME} ${SRC_LIST} ../prepareSE/sebuilder.cpp ${MOCSrcs} ${UISrcs} ${QM} ${QRC})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${SBF_LIBRARIES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} metis)
