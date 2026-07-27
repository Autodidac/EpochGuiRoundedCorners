cmake_minimum_required(VERSION 3.28)

project(EpochGuiRoundedCorners LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(NOT CMAKE_GENERATOR MATCHES "Ninja|Visual Studio")
    message(FATAL_ERROR "EpochGUI's C++23 module demo requires Ninja or Visual Studio.")
endif()

set(EPOCH_ENGINE_ROOT "" CACHE PATH "Path to EpochEngine's Engine directory")

set(_epoch_engine_candidates)
if(EPOCH_ENGINE_ROOT)
    list(APPEND _epoch_engine_candidates "${EPOCH_ENGINE_ROOT}")
endif()
list(APPEND _epoch_engine_candidates
    "${CMAKE_CURRENT_SOURCE_DIR}/../EpochEngine/Engine"
    "${CMAKE_CURRENT_SOURCE_DIR}/../EpochEngine"
)

set(_epoch_resolved_engine_root "")
foreach(_candidate IN LISTS _epoch_engine_candidates)
    get_filename_component(_candidate_abs "${_candidate}" ABSOLUTE)

    if(EXISTS "${_candidate_abs}/dep/EpochGui/modules/epoch.gui.ixx")
        set(_epoch_resolved_engine_root "${_candidate_abs}")
        break()
    endif()

    if(EXISTS "${_candidate_abs}/Engine/dep/EpochGui/modules/epoch.gui.ixx")
        set(_epoch_resolved_engine_root "${_candidate_abs}/Engine")
        break()
    endif()
endforeach()

if(NOT _epoch_resolved_engine_root)
    message(FATAL_ERROR
        "EpochGUI was not found. Pass -DEPOCH_ENGINE_ROOT=<EpochEngine>/Engine. "
        "Expected dep/EpochGui/modules/epoch.gui.ixx below that directory.")
endif()

set(EPOCH_ENGINE_ROOT "${_epoch_resolved_engine_root}" CACHE PATH
    "Path to EpochEngine's Engine directory" FORCE)

message(STATUS "EpochEngine root: ${EPOCH_ENGINE_ROOT}")

add_library(epoch_gui_rounded_rect STATIC
    src/rounded_rect.cpp
)

set_target_properties(epoch_gui_rounded_rect PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

target_sources(epoch_gui_rounded_rect PUBLIC
    FILE_SET epoch_gui_rounded_rect_modules TYPE CXX_MODULES
        BASE_DIRS
            "${EPOCH_ENGINE_ROOT}/dep/EpochGui/modules"
            "${CMAKE_CURRENT_SOURCE_DIR}/modules"
        FILES
            "${EPOCH_ENGINE_ROOT}/dep/EpochGui/modules/epoch.gui.ixx"
            "${CMAKE_CURRENT_SOURCE_DIR}/modules/epoch.gui.rounded_rect.ixx"
)

target_compile_features(epoch_gui_rounded_rect PUBLIC cxx_std_23)

add_library(epoch_gui_demo_renderer STATIC
    src/opengl_renderer.cpp
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

target_link_libraries(epoch_gui_demo_renderer PUBLIC
    epoch_gui_rounded_rect
)

target_compile_features(epoch_gui_demo_renderer PUBLIC cxx_std_23)

add_executable(epoch_gui_rounded_corners_demo
    src/main.cpp
)

set_target_properties(epoch_gui_rounded_corners_demo PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

target_link_libraries(epoch_gui_rounded_corners_demo PRIVATE
    epoch_gui_rounded_rect
)

target_compile_features(epoch_gui_rounded_corners_demo PRIVATE cxx_std_23)

if(WIN32)
    add_executable(epoch_gui_rounded_corners_app WIN32
        src/platform/windows_main.cpp
    )

    target_compile_definitions(epoch_gui_rounded_corners_app PRIVATE
        UNICODE
        _UNICODE
    )

    target_link_libraries(epoch_gui_rounded_corners_app PRIVATE
        epoch_gui_demo_renderer
        opengl32
    )
elseif(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 23)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    set(CMAKE_OBJCXX_EXTENSIONS OFF)

    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)

    add_executable(epoch_gui_rounded_corners_app MACOSX_BUNDLE
        src/platform/macos_main.mm
    )

    target_link_libraries(epoch_gui_rounded_corners_app PRIVATE
        epoch_gui_demo_renderer
        "${COCOA_FRAMEWORK}"
        "${OPENGL_FRAMEWORK}"
    )
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)

    add_executable(epoch_gui_rounded_corners_app
        src/platform/linux_main.cpp
    )

    target_link_libraries(epoch_gui_rounded_corners_app PRIVATE
        epoch_gui_demo_renderer
        OpenGL::GL
        X11::X11
    )
else()
    message(FATAL_ERROR "No native OpenGL host is defined for this platform.")
endif()

set_target_properties(epoch_gui_rounded_corners_app PROPERTIES
    OUTPUT_NAME "EpochGuiRoundedCorners"
)

target_compile_features(epoch_gui_rounded_corners_app PRIVATE cxx_std_23)

if(MSVC)
    foreach(_target IN ITEMS
        epoch_gui_rounded_rect
        epoch_gui_demo_renderer
        epoch_gui_rounded_corners_demo
        epoch_gui_rounded_corners_app)
        target_compile_options(${_target} PRIVATE /W4 /permissive-)
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    foreach(_target IN ITEMS
        epoch_gui_rounded_rect
        epoch_gui_demo_renderer
        epoch_gui_rounded_corners_demo
        epoch_gui_rounded_corners_app)
        target_compile_options(${_target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

if(WIN32)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT epoch_gui_rounded_corners_app)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(
        NAME epoch_gui_rounded_corners_demo
        COMMAND epoch_gui_rounded_corners_demo
            "${CMAKE_CURRENT_BINARY_DIR}/epochgui_rounded_corners_demo.svg"
    )
endif()
