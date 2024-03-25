#
# Copyright (c) 2020 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app_core)

# NORDIC SDK APP START
file(GLOB_RECURSE APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
target_sources(app PRIVATE ${APP_SOURCES})

# NORDIC SDK APP END

# Add project directories
include_directories(inc)

zephyr_library_include_directories(.)

# Add CMSIS library from nrfSDK
include_directories(..\\..\\..\\..\\modules\\hal\\cmsis\\CMSIS\\DSP\\Include)