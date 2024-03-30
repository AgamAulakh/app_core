# Include all source files in the src/core directory
file(GLOB_RECURSE CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Add the sources to the app target
target_sources(app PRIVATE ${CORE_SOURCES})
