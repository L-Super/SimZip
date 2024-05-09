cmake_minimum_required(VERSION 3.28)
project(SimZip)

set(CMAKE_CXX_STANDARD 17)

add_executable(SimZip main.cpp
        SimZip.cpp
        SimZip.h)

include(FetchContent)
FetchContent_Declare(miniz
        GIT_REPOSITORY https://github.com/richgel999/miniz.git
        GIT_TAG 3.0.2
        DOWNLOAD_EXTRACT_TIMESTAMP true)
FetchContent_MakeAvailable(miniz)

target_link_libraries(SimZip PRIVATE miniz)