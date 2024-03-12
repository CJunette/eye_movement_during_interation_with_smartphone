cmake_minimum_required(VERSION 3.5)
project(seeso)

set(CMAKE_CXX_STANDARD 14)

set(SEESO_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(SEESO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SEESO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/seeso)


set(SEESO_DLL
        "${SEESO_BIN_DIR}/seeso/seeso_core.dll"
        "${SEESO_BIN_DIR}/third-parties/libcrypto-1_1-x64.dll"
        "${SEESO_BIN_DIR}/third-parties/libcurl.dll"
        "${SEESO_BIN_DIR}/third-parties/libssl-1_1-x64.dll"
        "${SEESO_BIN_DIR}/third-parties/opencv_world410.dll"
        "${SEESO_BIN_DIR}/third-parties/tensorflowlite.dll"
        "${SEESO_BIN_DIR}/third-parties/tensorflowlite_c.dll"
        )
set(SEESO_DLL ${SEESO_DLL} PARENT_SCOPE)

if(WIN32)
    add_library(seeso STATIC
            ${SEESO_DIR}/error_reporter.cc
            ${SEESO_DIR}/gaze_tracker.cc
            ${SEESO_DIR}/framework/core_callback.cc
            ${SEESO_DIR}/util/display_w32.cc
            )

    message("SEESO dll: ${SEESO_DLL}")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(SEESO_CORE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/release/libseeso_core.dylib)
    else()
        set(SEESO_CORE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/debug/libseeso_core.dylib)
    endif()

    find_library(FoundationLib Foundation)
    find_library(FoundationLib CoreFoundation)
    find_library(FoundationLib CoreGraphics)
    find_library(FoundationLib Cocoa)

    add_library(seeso STATIC
            ${SEESO_DIR}/error_reporter.cc
            ${SEESO_DIR}/gaze_tracker.cc
            ${SEESO_DIR}/framework/core_callback.cc
            ${SEESO_DIR}/util/display_cocoa.mm
            )

    set_source_files_properties(${SEESO_DIR}/util/display_cocoa.mm PROPERTIES
            COMPILE_FLAGS "-x objective-c++")

    target_link_libraries(seeso PUBLIC
            ${SEESO_CORE_LIB}
            "-framework Foundation"
            "-framework CoreFoundation"
            "-framework CoreGraphics"
            "-framework Cocoa")

elseif(UNIX)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(SEESO_CORE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/release/libseeso_core.so)
    else()
        set(SEESO_CORE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/debug/libseeso_core.so)
    endif()

    add_library(seeso STATIC
            ${SEESO_DIR}/error_reporter.cc
            ${SEESO_DIR}/gaze_tracker.cc
            ${SEESO_DIR}/framework/core_callback.cc
            )

    target_link_libraries(seeso PUBLIC ${SEESO_CORE_LIB})

else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

target_include_directories(seeso PUBLIC ${SEESO_INCLUDE_DIR})
#add_dependencies(seeso seeso_core)
