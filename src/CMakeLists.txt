project(antiview_src)

add_executable(off_color off_color.cc)
set_all_compiler_settings(off_color)
target_link_libraries(off_color PRIVATE antiprism)

add_executable(off2obj off2obj.cc)
set_all_compiler_settings(off2obj)
target_link_libraries(off2obj PRIVATE antiprism)

add_executable(off_util off_util.cc help.h)
set_all_compiler_settings(off_util)
target_link_libraries(off_util PRIVATE antiprism)
