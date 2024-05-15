cmake_minimum_required(VERSION 3.28)
project(SimZip)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

option(ENABLE_TESTS "Build the test" ON)

add_library(SimZip
        SimZip.cpp SimZip.h)

include(FetchContent)
FetchContent_Declare(miniz
        GIT_REPOSITORY https://github.com/richgel999/miniz.git
        GIT_TAG 3.0.2
        DOWNLOAD_EXTRACT_TIMESTAMP true)
FetchContent_MakeAvailable(miniz)

target_link_libraries(SimZip PRIVATE miniz)

if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()