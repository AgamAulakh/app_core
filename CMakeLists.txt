#
# Copyright (c) 2020 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app_core)

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.cpp)
# NORDIC SDK APP END

zephyr_library_include_directories(.)

# Add project directories
add_subdirectory(src)

# Add CMSIS library from nrfSDK
include_directories(..\\..\\..\\..\\modules\\hal\\cmsis\\CMSIS\\DSP\\Include)

# Add Brainflow library from external folder
# set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libraries)
# target_precompile_headers(app_core PRIVATE ${LIB_DIR}/Brainflow/compiled/Release/)

# Add external libraries
# set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libraries)
# add_subdirectory(${LIB_DIR}/CMSIS-DSP/Include subproject/grzeslib EXCLUDE_FROM_ALL)
# target_include_directories(app_core PRIVATE ${LIB_DIR}/CMSIS-DSP/Include)

# add_subdirectory(${LIB_DIR}/CMSIS-DSP/Include subproject/grzeslib EXCLUDE_FROM_ALL)
# add_subdirectory(${LIB_DIR}/CMSIS-DSP/Source bin_dsp)

# include_directories("${PROJECT_SOURCE_DIR}/libraries/Brainflow/compiled/Release/")
# add_subdirectory(libraries/Brainflow/compiled/Release/)
# target_link_libraries(app_core libraries/Brainflow/compiled/Release/)

# add_library(brainflow STATIC IMPORTED GLOBAL)
# set_target_properties(brainflow PROPERTIES IMPORTED_LOCATION ${LIB_DIR}/Brainflow/compiled/Release/)
# target_link_libraries(app PUBLIC brainflow)

