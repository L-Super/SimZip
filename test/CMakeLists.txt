include(FetchContent)
FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.6.0.tar.gz
        GIT_TAG 4e8d92b #v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(SimZipTest test.cpp)
target_link_libraries(SimZipTest PRIVATE SimZip Catch2::Catch2WithMain)

add_test(NAME SimZipTest
        COMMAND $<TARGET_FILE:SimZipTest> --success
)

