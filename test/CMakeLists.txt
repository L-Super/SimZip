include(FetchContent)
FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.6.0.tar.gz
        GIT_TAG 4e8d92b #v3.6.0
)
FetchContent_MakeAvailable(Catch2)

set(TARGET_NAME SimZipTest)
set(SRC test.cpp)

add_executable(${TARGET_NAME} ${SRC})
target_link_libraries(${TARGET_NAME} PRIVATE SimZip Catch2::Catch2WithMain)

add_test(NAME ${TARGET_NAME}
        COMMAND $<TARGET_FILE:SimZipTest> --success
)

