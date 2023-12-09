project(qhull)

add_library(${PROJECT_NAME})

file(GLOB Headers *.h)
file(GLOB Sources *.cc)

target_sources(${PROJECT_NAME} PRIVATE ${Headers} ${Sources})

set_all_compiler_settings(${PROJECT_NAME})

