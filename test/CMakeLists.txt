include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(SimZipTest test.cpp)
target_link_libraries(SimZipTest PRIVATE SimZip Catch2::Catch2WithMain)

add_test(NAME SimZipTest
        COMMAND $<TARGET_FILE:SimZipTest> --success
)

