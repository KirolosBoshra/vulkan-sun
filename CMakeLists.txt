cmake_minimum_required(VERSION 3.20)
project(vulkan_sun LANGUAGES C)

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

add_executable(vulkan_sun ${SOURCES} ${HEADERS})

# auxiliary sources shared by test executables (everything under src/ except main.c)
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS src/*.c src/*.h)
list(REMOVE_ITEM LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")

target_include_directories(vulkan_sun PRIVATE src ${GLFW_INCLUDE_DIR})
target_link_libraries(vulkan_sun PRIVATE Vulkan::Vulkan ${GLFW_LIBRARY} gdi32)
target_compile_definitions(vulkan_sun PRIVATE
    $<$<CONFIG:Debug>:LOGGING>
    SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_SOURCE_DIR}/shaders/*.frag")

set(SHADER_BINARIES)
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(NAME ${SHADER} NAME)
    set(SPV "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.spv")
    list(APPEND SHADER_BINARIES ${SPV})

    add_custom_command(
        OUTPUT ${SPV}
        COMMAND Vulkan::glslc -o "${SPV}" "${SHADER}"
        DEPENDS ${SHADER}
        COMMENT "Compiling ${NAME}")
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_BINARIES})
add_dependencies(vulkan_sun shaders)

if(GLFW_DLL)
    add_custom_command(TARGET vulkan_sun POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLFW_DLL}"
            "$<TARGET_FILE_DIR:vulkan_sun>"
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

option(VS_BUILD_TESTS "Build tests" ON)

if(VS_BUILD_TESTS)
    enable_testing()

    add_executable(test_math tests/test_math.c)
    target_include_directories(test_math PRIVATE src)
    target_link_libraries(test_math PRIVATE m)
    add_test(NAME math COMMAND test_math)

    add_executable(test_dynarr tests/test_dynarr.c)
    target_include_directories(test_dynarr PRIVATE src)
    target_link_libraries(test_dynarr PRIVATE m)
    add_test(NAME dynarr COMMAND test_dynarr)

    add_executable(test_camera tests/test_camera.c)
    target_include_directories(test_camera PRIVATE src)
    target_link_libraries(test_camera PRIVATE m)
    add_test(NAME camera COMMAND test_camera)

    add_executable(test_vertex tests/test_vertex.c)
    target_include_directories(test_vertex PRIVATE src ${Vulkan_INCLUDE_DIRS}
                                ${GLFW_INCLUDE_DIR})
    target_link_libraries(test_vertex PRIVATE m)
    add_test(NAME vertex COMMAND test_vertex)

    add_executable(test_buffer tests/test_buffer.c ${LIB_SOURCES})
    target_include_directories(test_buffer PRIVATE src ${Vulkan_INCLUDE_DIRS}
                                ${GLFW_INCLUDE_DIR})
    target_link_libraries(test_buffer PRIVATE Vulkan::Vulkan ${GLFW_LIBRARY}
                            gdi32)
    add_test(NAME buffer COMMAND test_buffer)

    add_executable(test_resource tests/test_resource.c ${LIB_SOURCES})
    target_include_directories(test_resource PRIVATE src ${Vulkan_INCLUDE_DIRS}
                                ${GLFW_INCLUDE_DIR})
    target_link_libraries(test_resource PRIVATE Vulkan::Vulkan ${GLFW_LIBRARY}
                            gdi32)
    add_test(NAME resource COMMAND test_resource)

    add_executable(test_depng tests/test_depng.c)
    target_include_directories(test_depng PRIVATE src)
    target_link_libraries(test_depng PRIVATE m)
    target_compile_definitions(test_depng PRIVATE
        ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")
    add_test(NAME depng COMMAND test_depng)

    add_executable(test_texture tests/test_texture.c ${LIB_SOURCES})
    target_include_directories(test_texture PRIVATE src ${Vulkan_INCLUDE_DIRS}
                                ${GLFW_INCLUDE_DIR})
    target_link_libraries(test_texture PRIVATE Vulkan::Vulkan ${GLFW_LIBRARY}
                            gdi32)
    target_compile_definitions(test_texture PRIVATE
        ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")
    add_test(NAME texture COMMAND test_texture)

    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
        USES_TERMINAL)
    add_dependencies(check test_math test_dynarr test_camera test_vertex test_buffer test_resource test_depng test_texture)
endif()
