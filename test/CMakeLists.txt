include(FetchContent)
FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.6.0.tar.gz
        GIT_TAG 4e8d92b #v3.6.0
)
FetchContent_MakeAvailable(Catch2)

set(TARGET_NAME SimZipTest)
set(SRC test.cpp)

if (MSVC)
    # 清单文件的输出路径
    set(MANIFEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.manifest")
    # 使用 configure_file 生成清单文件，将 TARGET_NAME 替换为实际目标名称
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app_manifest.xml.in ${MANIFEST_FILE} @ONLY)
    set(SRC ${SRC} ${MANIFEST_FILE})
endif ()

add_executable(${TARGET_NAME} ${SRC})
target_link_libraries(${TARGET_NAME} PRIVATE SimZip Catch2::Catch2WithMain)

if (MSVC)
    target_compile_options(${TARGET_NAME} PUBLIC /utf-8)
endif ()

add_test(NAME ${TARGET_NAME}
        COMMAND $<TARGET_FILE:SimZipTest> --success
)

