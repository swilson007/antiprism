# Sets any MacOS specific global cmake variables
function(set_platform_global_settings)
    set(CMAKE_CXX_FLAGS_RELEASE "" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_DEBUG "" PARENT_SCOPE)
endfunction()

################################################################################
# Subprojects call this function to setup all compiler settings
# Example: set_all_compiler_settings(${PROJECT_NAME})
function(set_os_compiler_settings target)
    get_target_property(targetType ${target} TYPE)

    # Handle INTERFACE targets
    if(targetType STREQUAL "INTERFACE_LIBRARY")
    else()
        target_link_options(${target} PRIVATE
                $<$<CONFIG:DebugSanitize>:-fsanitize=address>)
        target_compile_options(${target} PRIVATE
                -Wno-deprecated-declarations
                $<$<CONFIG:Debug>:-g>
                $<$<CONFIG:Release>:-O2 -DNDEBUG>)
    endif()
endfunction()
