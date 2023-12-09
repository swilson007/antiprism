function(set_global_settings)
    # TODO As needed
endfunction()

################################################################################
# Sets any compiler settings that will be shared across OS's
#
# Parameters:
#  <target-name:required>
function(set_common_compiler_settings target)
    # Handle interface targets
    get_target_property(targetType ${target} TYPE)
    if(targetType STREQUAL "INTERFACE_LIBRARY")
        set(externalVis INTERFACE)
    else()
        set(externalVis PUBLIC)
    endif()

    set_target_properties(${target} PROPERTIES
            CXX_EXTENSIONS OFF
            CXX_STANDARD 14
            CXX_STANDARD_REQUIRED TRUE)

    # This is for our generated config file
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    target_compile_definitions(${target} ${externalVis} HAVE_CONFIG_H)
endfunction()

