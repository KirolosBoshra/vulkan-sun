cmake_minimum_required(VERSION 3.20)
project(vulken_sun LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)

if(DEFINED ENV{GLFW_ROOT})
    set(_GLFW_ROOT "$ENV{GLFW_ROOT}")
elseif(WIN32 AND DEFINED ENV{HOME})
    set(_GLFW_ROOT "$ENV{HOME}/scoop/apps/glfw/current")
endif()

find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h
    HINTS "${_GLFW_ROOT}"
    PATH_SUFFIXES include
)
find_library(GLFW_LIBRARY
    NAMES glfw3dll glfw3
    HINTS "${_GLFW_ROOT}"
    PATH_SUFFIXES lib lib-mingw-w64 lib-vc2022
)
find_file(GLFW_DLL glfw3.dll
    HINTS "${_GLFW_ROOT}"
    PATH_SUFFIXES lib lib-mingw-w64 lib-vc2022 bin
)

if(NOT GLFW_INCLUDE_DIR OR NOT GLFW_LIBRARY)
    message(FATAL_ERROR "GLFW not found. Install GLFW or set GLFW_ROOT to its directory")
endif()

mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY GLFW_DLL)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.c)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS src/*.h)

add_executable(vulken_sun ${SOURCES} ${HEADERS})

target_include_directories(vulken_sun PRIVATE src ${GLFW_INCLUDE_DIR})
target_link_libraries(vulken_sun PRIVATE Vulkan::Vulkan ${GLFW_LIBRARY} gdi32)

if(GLFW_DLL)
    add_custom_command(TARGET vulken_sun POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLFW_DLL}"
            "$<TARGET_FILE_DIR:vulken_sun>"
        COMMENT "Copy glfw3.dll to output dir")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/compile_commands.json")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
        ERROR_QUIET)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/compile_commands.json")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json")
    endif()
endif()
