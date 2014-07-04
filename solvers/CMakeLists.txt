option(BUILD_EIGENSOLVER "Build eigenSolver project" ON)

if(BUILD_EIGENSOLVER)
    message("Configuring eigenSolver")
    add_subdirectory(eigenSolver)
endif()
