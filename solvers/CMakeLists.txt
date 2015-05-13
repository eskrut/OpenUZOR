option(BUILD_STATICSESOLVER "Build staticSESolver project" ON)
option(BUILD_EIGENSOLVER "Build eigenSolver project" ON)

if(BUILD_STATICSESOLVER)
    message("Configuring staticSESolver")
    add_subdirectory(staticSESolver)
endif()

if(BUILD_EIGENSOLVER)
    message("Configuring eigenSolver")
    add_subdirectory(eigenSolver)
endif()
