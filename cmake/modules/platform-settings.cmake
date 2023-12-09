################################################################################
# This file load per-platform settings that aren't target dependent. Also contains
# common functions that are used across platforms.
################################################################################

include(sys-defines)
include(global-settings)

if(Macos)
    include(macos-settings)
elseif(Linux)
    message(FATAL_ERROR "Linux TODO")
elseif(Windows)
    include(windows-settings)
else()
    message(FATAL_ERROR "Unknown OS")
endif()

set_global_settings()
set_platform_global_settings()

################################################################################
# Sets common and compiler-specific settings for a target.
#
# Named Parameters:
#  <target-name:required>
function(set_all_compiler_settings target)
    set_common_compiler_settings(${target})
    set_os_compiler_settings(${target})
endfunction()

function(set_bgfx_compiler_settings target)
    set_common_compiler_settings(${target})
    set_bgfx_os_compiler_settings(${target})
endfunction()
