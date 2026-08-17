cmake_minimum_required(VERSION 3.24)
set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(nebula)

add_subdirectory(lib/DarkMatter)
add_subdirectory(lib/box3d)

set(SOURCES main.c application.c renderer.c random_gen.c instances.c gui.c)

add_definitions(-DCGLM_FORCE_DEPTH_ZERO_TO_ONE)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC lib lib/cglm/include lib/box3d/include)
target_link_libraries(${PROJECT_NAME} PUBLIC DarkMatter box3d)
