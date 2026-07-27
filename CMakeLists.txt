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
    add_executable(epoch_gui_rounded_corners_windows WIN32
        src/windows_app.cpp
    )

    set_target_properties(epoch_gui_rounded_corners_windows PROPERTIES
        CXX_SCAN_FOR_MODULES ON
        OUTPUT_NAME "EpochGuiRoundedCorners"
    )

    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT epoch_gui_rounded_corners_windows)

    target_compile_definitions(epoch_gui_rounded_corners_windows PRIVATE
        UNICODE
        _UNICODE
    )

    target_compile_features(epoch_gui_rounded_corners_windows PRIVATE cxx_std_23)

    target_link_libraries(epoch_gui_rounded_corners_windows PRIVATE
        epoch_gui_rounded_rect
        d2d1
        dwrite
    )
endif()

if(MSVC)
    target_compile_options(epoch_gui_rounded_rect PRIVATE /W4 /permissive-)
    target_compile_options(epoch_gui_rounded_corners_demo PRIVATE /W4 /permissive-)
    if(TARGET epoch_gui_rounded_corners_windows)
        target_compile_options(epoch_gui_rounded_corners_windows PRIVATE /W4 /permissive-)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(epoch_gui_rounded_rect PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(epoch_gui_rounded_corners_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(
        NAME epoch_gui_rounded_corners_demo
        COMMAND epoch_gui_rounded_corners_demo
            "${CMAKE_CURRENT_BINARY_DIR}/epochgui_rounded_corners_demo.svg"
    )
endif()
