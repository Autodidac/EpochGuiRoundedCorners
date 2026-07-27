cmake_minimum_required(VERSION 3.28)

project(EpochGuiDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(NOT CMAKE_GENERATOR MATCHES "Ninja|Visual Studio")
    message(FATAL_ERROR "EpochGui_Demo requires Ninja or Visual Studio for C++23 modules.")
endif()

set(EPOCHGUI_ROOT "" CACHE PATH "Path to the standalone EpochGui repository")

set(_epochgui_candidates)
if(EPOCHGUI_ROOT)
    list(APPEND _epochgui_candidates "${EPOCHGUI_ROOT}")
endif()
list(APPEND _epochgui_candidates
    "${CMAKE_CURRENT_SOURCE_DIR}/../EpochGui"
    "${CMAKE_CURRENT_SOURCE_DIR}/EpochGui"
)

set(_epochgui_resolved_root "")
foreach(_candidate IN LISTS _epochgui_candidates)
    get_filename_component(_candidate_abs "${_candidate}" ABSOLUTE)
    if(EXISTS "${_candidate_abs}/CMakeLists.txt"
        AND EXISTS "${_candidate_abs}/modules/epoch.gui.ixx"
        AND EXISTS "${_candidate_abs}/modules/epoch.gui.rounded_rect.ixx"
        AND EXISTS "${_candidate_abs}/modules/epoch.gui.input.ixx")
        set(_epochgui_resolved_root "${_candidate_abs}")
        break()
    endif()
endforeach()

if(NOT _epochgui_resolved_root)
    message(FATAL_ERROR
        "Standalone EpochGui with rounded rectangles and fallback input was not found. "
        "Pass -DEPOCHGUI_ROOT=<path-to-Autodidac/EpochGui>.")
endif()

set(EPOCHGUI_ROOT "${_epochgui_resolved_root}" CACHE PATH
    "Path to the standalone EpochGui repository" FORCE)
set(EPOCHGUI_SOURCE_ROOT "${EPOCHGUI_ROOT}" CACHE PATH
    "EpochGui source root used by the demo" FORCE)
set(EPOCHGUI_ENABLE_ROUNDED_RECT ON CACHE BOOL
    "Enable optional EpochGui rounded-rectangle geometry for comparison" FORCE)
set(EPOCHGUI_ENABLE_INPUT ON CACHE BOOL
    "Enable optional EpochGui fallback input for this standalone demo" FORCE)

message(STATUS "EpochGui root: ${EPOCHGUI_ROOT}")
message(STATUS "EpochGui fallback input: enabled for demo")

add_subdirectory(
    "${EPOCHGUI_ROOT}"
    "${CMAKE_CURRENT_BINARY_DIR}/EpochGui"
)

add_library(epoch_gui_demo_renderer STATIC
    src/live_renderer.cpp
    src/app_bridge.cpp
)

set_target_properties(epoch_gui_demo_renderer PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

target_sources(epoch_gui_demo_renderer PUBLIC
    FILE_SET epoch_gui_demo_renderer_modules TYPE CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/modules"
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/modules/epoch.gui.demo.opengl.ixx"
)

target_include_directories(epoch_gui_demo_renderer PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(epoch_gui_demo_renderer PUBLIC EpochGui)
target_compile_features(epoch_gui_demo_renderer PUBLIC cxx_std_23)

if(WIN32)
    enable_language(RC)

    add_executable(epoch_gui_demo_app WIN32
        src/platform/windows_main.cpp
        src/platform/windows_resources.rc
        src/platform/windows_message_coords.hpp
    )

    target_compile_definitions(epoch_gui_demo_app PRIVATE
        UNICODE
        _UNICODE
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
    )

    target_link_libraries(epoch_gui_demo_app PRIVATE
        epoch_gui_demo_renderer
        dwmapi
        opengl32
    )

    if(MSVC)
        target_compile_options(epoch_gui_demo_app PRIVATE
            "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows_message_coords.hpp"
        )
    endif()
elseif(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 23)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    set(CMAKE_OBJCXX_EXTENSIONS OFF)

    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)

    add_executable(epoch_gui_demo_app MACOSX_BUNDLE
        src/platform/macos_main.mm
    )

    target_link_libraries(epoch_gui_demo_app PRIVATE
        epoch_gui_demo_renderer
        "${COCOA_FRAMEWORK}"
        "${OPENGL_FRAMEWORK}"
    )
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)

    add_executable(epoch_gui_demo_app
        src/platform/linux_glx_main.cpp
    )

    target_link_libraries(epoch_gui_demo_app PRIVATE
        epoch_gui_demo_renderer
        OpenGL::GL
        X11::X11
    )
else()
    message(FATAL_ERROR "No native OpenGL host is defined for this platform.")
endif()

set_target_properties(epoch_gui_demo_app PROPERTIES
    OUTPUT_NAME "EpochGuiDemo"
)

target_compile_features(epoch_gui_demo_app PRIVATE cxx_std_23)

set(_epoch_gui_targets
    epoch_gui_demo_renderer
    epoch_gui_demo_app
)

if(MSVC)
    foreach(_target IN LISTS _epoch_gui_targets)
        target_compile_options(${_target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    foreach(_target IN LISTS _epoch_gui_targets)
        target_compile_options(${_target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

if(WIN32)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT epoch_gui_demo_app)
endif()
