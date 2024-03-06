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

# COMPILER AND LINKER OPTIONS

# Add project directories
zephyr_library_include_directories(.)
add_subdirectory(src)

# Add CMSIS library from nrfSDK
include_directories(..\\..\\..\\..\\modules\\hal\\cmsis\\CMSIS\\DSP\\Include)