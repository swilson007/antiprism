################################################################################
# This will setup OS and compiler based variables that can be used to direct per-platform
# cmake file loading and if/then choices.

# Allow 'if (MACOS)' style constructs
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # If we ever support iOS, we'll need to have a -DIOS and do some
    # testing here because you can't tell from cmake
    set(Macos TRUE)
    set(Macosx 1)
    set(MacosSW TRUE)
    set(Posix TRUE)
    set(OsBuildDir "macos")
    message("Macos=${Macos}")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(Linux TRUE)
    set(Posix TRUE)
    set(OsBuildDir "linux64")
    message("Linux=${Linux}")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(Windows TRUE)
    set(Posix FALSE)
    set(OsBuildDir "win64")    # TODO version?
    message("Windows=${Windows}")
endif()

# Friendly compiler names
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(Clang TRUE)
    message("Clang=${Clang}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(Clang TRUE)
    message("Clang=${Clang}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(Gcc TRUE)
    message("Gcc=${Gcc}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(Msvc TRUE)
    message("Msvc=${Msvc}")
endif()

# Other system wide globals
