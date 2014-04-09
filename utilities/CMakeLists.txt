option(BUILD_PREPARE_SE "Build prepareSE project" ON)
option(BUILD_SBFTOVTK "Build sbfToVtk project" ON)
option(BUILD_QSBFTOVTK "Build qSbfToVtk project" ON)

if(BUILD_PREPARE_SE)
    message("Configuring prepareSE")
    add_subdirectory(prepareSE)
endif()

if(BUILD_SBFTOVTK)
    message("Configuring sbfToVtk")
    add_subdirectory(sbfToVtk)
endif()

if(BUILD_QSBFTOVTK)
    message("Configuring qSbfToVtk")
    add_subdirectory(qSbfToVtk)
endif()
