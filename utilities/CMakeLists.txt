option(BUILD_PREPARE_SE "Build prepareSE project" ON)
option(BUILD_QSBFTOVTK "Build qSbfToVtk project" ON)

if(BUILD_PREPARE_SE)
    add_subdirectory(prepareSE)
endif()

if(BUILD_QSBFTOVTK)
    add_subdirectory(qSbfToVtk)
endif()