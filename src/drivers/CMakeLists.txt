# Include all source files in the src/drivers directory
file(GLOB_RECURSE DRIVER_CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE DRIVER_C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

# Add the sources to the app target
target_sources(app PRIVATE ${DRIVER_CPP_SOURCES})
target_sources(app PRIVATE ${DRIVER_C_SOURCES})
