cmake_minimum_required(VERSION 3.25)
project(SimZip
        VERSION 0.9.0
        DESCRIPTION "A minimal C++20 zip library for zip compression and decompression."
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SIMZIP_STATIC "Build SimZIP as static library" ON)
option(ENABLE_TESTS "Build the test" OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

if (BUILD_SIMZIP_STATIC)
    set(SIMZIP_LIB_TYPE STATIC)
else ()
    set(SIMZIP_LIB_TYPE SHARED)
endif ()

add_library(SimZip ${SIMZIP_LIB_TYPE}
        SimZip.cpp SimZip.h)
add_library(SimZip::SimZip ALIAS SimZip)

if (MSVC)
    target_compile_options(SimZip PRIVATE /utf-8)
    target_compile_definitions(SimZip PRIVATE UNICODE _UNICODE)
endif ()

if (NOT ${BUILD_SIMZIP_STATIC})
    set_target_properties(${PROJECT_NAME} PROPERTIES
            DEFINE_SYMBOL "SIMZIP_EXPORTS" # Override CMake's default export macro (simzip_EXPORTS) with a custom one
    )
    message(STATUS "Build ${PROJECT_NAME} shared lib")
else ()
    target_compile_definitions(${PROJECT_NAME} PUBLIC SIMZIP_STATIC)
    message(STATUS "Build ${PROJECT_NAME} static lib")
endif ()

include(FetchContent)
FetchContent_Declare(miniz
        GIT_REPOSITORY https://github.com/richgel999/miniz.git
        GIT_TAG 1ff82be7d67f5c2f8b5497f538eea247861e0717)
FetchContent_MakeAvailable(miniz)
target_include_directories(SimZip PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(SimZip PRIVATE miniz)

include(GNUInstallDirs)

if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()
