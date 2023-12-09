# Sets any MacOS specific global cmake variables
function(set_platform_global_settings)
    set(BgfxShaderPlatform windows PARENT_SCOPE)
    set(BgfxShaderApi dx12 PARENT_SCOPE)
    set(BgfxShaderProfileVert vs_5_0 PARENT_SCOPE)
    set(BgfxShaderProfileFrag ps_5_0 PARENT_SCOPE)
    set(BgfxShaderProfileComp cs_5_0 PARENT_SCOPE)
endfunction()

################################################################################
# Subprojects call this function to setup all compiler settings
#
# Example: set_all_compiler_settings(${PROJECT_NAME})
#
# Named Parameters:
#  TARGET <target:required>
function(set_os_compiler_settings target)
    # Determine the type of the target
    get_target_property(targetType ${target} TYPE)

    # If the target is an INTERFACE target, we can't do the rest so bail out now
    if (targetType STREQUAL "INTERFACE_LIBRARY")
        return()
    endif ()

    target_compile_definitions(${target} PUBLIC
            SDL_MAIN_HANDLED
            NOMINMAX UNICODE _UNICODE WIN32_LEAN_AND_MEAN)

    # SCW Research note: To disable system include warnings, see https://devblogs.microsoft.com/cppblog/broken-warnings-theory/
    # and https://gitlab.kitware.com/cmake/cmake/-/issues/17904
    # It doesn't appear to be supported properly yet as of 9/2021
    target_compile_options(${target} PRIVATE
            /MP
            /Zc:__cplusplus # Ensures __cplusplus is correct
            /W3 /WX # /W4 producing strange "unreachable code C4702" warnings
            /wd4068 # Ignore unknown pragmas
            /wd4127 # consider 'if constexpr'
            /wd4201 # Non-standard extension: nameless struct. (In glm header)
            /wd4324 # structure padded due to alignas()... that's the whole point of it
            /wd4996 # c++17 deprecated stuff triggered by abseil include
            /wd4389 # gtest has some issues with this
            $<$<CONFIG:Debug>:/wd4100> # unused-vars
            $<$<CONFIG:Debug>:/wd4189> # unused-vars
            $<$<CONFIG:Debug>:/wd4702> # unused-vars
            )

endfunction()
