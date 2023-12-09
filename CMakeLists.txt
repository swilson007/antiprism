cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0092 NEW)  # No default MSVC warning flags
cmake_policy(SET CMP0022 NEW)  # Target link headers

#set(CMAKE_VERBOSE_MAKEFILE ON)
project(antiprism)

# OS and compiler settings
list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")
include(platform-settings)

# config file
CONFIGURE_FILE(sw_config.h config.h)

# Load all the goo for everything else
add_subdirectory(base)
add_subdirectory(src)
