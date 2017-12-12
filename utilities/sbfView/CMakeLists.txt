project(sbfView)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

add_subdirectory(src)

find_package(sbf REQUIRED)
include_directories(${SBF_INCLUDE_DIRS})

find_package(VTK COMPONENTS
  vtkCommonCore
  vtkGUISupportQt
  vtkIOInfovis
  vtkViewsInfovis
REQUIRED NO_MODULE)
include(${VTK_USE_FILE})

set(Srcs ${SRC_LIST})
set(Hdrs "")
set(QRCs ${RC_LIST})
set(MOC_Hdrs ${MOC_LIST})

set(CMAKE_AUTOMOC ON)

if(VTK_QT_VERSION VERSION_GREATER "4")
  find_package(Qt5Widgets REQUIRED)

  qt5_add_resources(QRC_Srcs ${QRCs} )

  add_executable(${PROJECT_NAME} #MACOSX_BUNDLE
    src/main.cpp ${Srcs} ${Hdrs} ${MOC_Hdrs} ${QRC_Srcs})
  qt5_use_modules(${PROJECT_NAME} Core Gui Widgets
                  OpenGL OpenGLExtensions)
  target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
else()
endif()

target_link_libraries(${PROJECT_NAME} ${SBF_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
