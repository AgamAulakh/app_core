# Include source files in the src directory
file(GLOB_RECURSE APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
target_sources(app PRIVATE ${APP_SOURCES})

# Add the core directory
add_subdirectory(core)
add_subdirectory(drivers)
