project(antiprism_lib)

add_library(antiprism)

file(GLOB Headers *.h)
file(GLOB Sources *.cc)

target_sources(antiprism PRIVATE ${Headers} ${Sources})

set_all_compiler_settings(antiprism)
target_link_libraries(antiprism PRIVATE muparser qhull tesselator)

add_subdirectory(muparser)
add_subdirectory(qhull)
add_subdirectory(tesselator)
