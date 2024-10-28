cmake_minimum_required(VERSION 3.25)
project(SimZip
        VERSION 0.9.0
        DESCRIPTION "A minimal C++17 zip library for zip compression and decompression."
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

option(ENABLE_TESTS "Build the test" OFF)

add_library(SimZip
        SimZip.cpp SimZip.h)
add_library(SimZip::SimZip ALIAS SimZip)

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
