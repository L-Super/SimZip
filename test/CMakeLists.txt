# 定义一个函数来为任意目标生成清单文件
function(add_utf8_manifest TARGET)
    # 清单文件的输出路径
    set(MANIFEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.manifest")

    # 使用 configure_file 生成清单文件，将 TARGET_NAME 替换为实际目标名称
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app_manifest.xml.in ${MANIFEST_FILE} @ONLY)

    # 将清单文件添加到目标的链接命令中
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MANIFEST_FILE}" "$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.exe.manifest"
            COMMENT "Adding UTF-8 manifest to ${TARGET}")
endfunction()

include(FetchContent)
FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.6.0.tar.gz
        GIT_TAG 4e8d92b #v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(SimZipTest test.cpp)
target_link_libraries(SimZipTest PRIVATE SimZip Catch2::Catch2WithMain)

if (MSVC)
    target_compile_options(SimZipTest PUBLIC /utf-8)
    add_utf8_manifest(SimZipTest)
endif ()

add_test(NAME SimZipTest
        COMMAND $<TARGET_FILE:SimZipTest> --success
)

