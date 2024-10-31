#include "../SimZip.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporters_all.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <clocale>
#include <Windows.h>
#endif

namespace fs = std::filesystem;

namespace {
    const std::string enFilename{"data.txt"};
    const std::string zhFileName{"文件.txt"};
}// namespace

void generateData(const fs::path& filename)
{
#if _WIN32
    // 设置标准库调用系统 API 所用的编码，用于 fopen，ifstream 等函数
    setlocale(LC_ALL, ".utf-8");
    // 设置控制台输出编码，或者 system("chcp 65001") 也行，这里 CP_UTF8 = 65001
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::ofstream file(filename);
    if (file.is_open()) {
        for (auto i = 0; i < 10; i++) { file << "this is data for test.\n"; }
    }
    file.close();
}

void clear()
{
    fs::remove(enFilename);
    fs::remove(zhFileName);
    fs::remove_all("test/");
}

class CleanupListener : public Catch::EventListenerBase {
public:
    using EventListenerBase::EventListenerBase;

    void testRunEnded(Catch::TestRunStats const& testRunStats) override
    {
        // Perform cleaning after the test run is completed
        // 测试运行结束后执行清理
        clear();
    }
};

// register listener
//CATCH_REGISTER_LISTENER(CleanupListener)

TEST_CASE("create zip", "[create_zip]")
{
    fs::create_directory("test");
    generateData(enFilename);
    SECTION("create_zip1")
    {
        SimZip zip("test/test1.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename) == true);
        REQUIRE(zip.add(enFilename, "folder/rename.txt") == true);
        REQUIRE(zip.add("empty.txt") == false);
        zip.save();
    }

    SECTION("create_zip2")
    {
        SimZip zip("test/test2.zip");
        zip.setmode(SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename) == true);
        REQUIRE(zip.add(enFilename, "folder/rename.txt") == true);
        REQUIRE(zip.add("empty.txt") == false);
        zip.save();
    }
}

TEST_CASE("create unicode zip", "[create_zip2]")
{
    fs::create_directory("test");

    SECTION("create_unicode_zip1")
    {
        generateData(enFilename);

        std::string zipName{"test/压缩包.zip"};
        SimZip zip(zipName, SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename) == true);
        REQUIRE(zip.add(enFilename, "folder/rename.txt") == true);
        REQUIRE(zip.add("empty.txt") == false);
        zip.save();
        REQUIRE(fs::exists(zipName));
    }

    SECTION("create_unicode_zip2")
    {
        generateData(zhFileName);
        SimZip zip("test/test-unicode.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(zhFileName) == true);
        REQUIRE(zip.add(zhFileName, "folder/文件.txt") == true);
        REQUIRE(zip.add(zhFileName, "文件夹/文件.txt") == true);
        REQUIRE(zip.add("不存在文件.txt") == false);
        zip.save();
    }
}

TEST_CASE("extract zip", "[extract_zip]")
{
    const std::string outputPath{"test/output"};
    SECTION("Extract single file from zip")
    {
        SimZip zip("test/test1.zip", SimZip::OpenMode::Read);
        zip.extract(enFilename, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + enFilename));
        fs::remove_all(outputPath);
    }

    SECTION("Extract all files from zip")
    {
        SimZip zip("test/test1.zip", SimZip::OpenMode::Read);
        zip.extractall(outputPath);
        std::vector<std::string> expected_files = {enFilename, "folder/rename.txt"};
        for (const auto& file: expected_files) { REQUIRE(fs::exists(outputPath + "/" + file)); }
        fs::remove_all(outputPath);
    }
}

TEST_CASE("extract unicode zip", "[extract_zip2]")
{
    SECTION("Extract the zip with unicode name")
    {
        const std::string outputPath{"test/output"};
        SimZip zip("test/压缩包.zip", SimZip::OpenMode::Read);
        zip.extract(enFilename, outputPath);
        for (const auto& file: fs::directory_iterator(outputPath)) {
            std::cout << "extracted file:" << file.path() << std::endl;
        }
        REQUIRE(fs::exists(outputPath + "/" + enFilename));
    }

    SECTION("Extract single unicode file from zip")
    {
        const std::string outputPath{"test/output2"};
        SimZip zip("test/test-unicode.zip", SimZip::OpenMode::Read);
        zip.extract(zhFileName, outputPath);
        for (const auto& file: fs::directory_iterator(outputPath)) {
            std::cout << "extracted file:" << file.path() << std::endl;
        }
        REQUIRE(fs::exists(outputPath + "/" + zhFileName));
    }

    SECTION("Extract all unicode files from zip")
    {
        const std::string outputPath{"test/output3"};
        SimZip zip("test/test-unicode.zip", SimZip::OpenMode::Read);
        zip.extractall(outputPath);
        std::vector<std::string> expected_files = {zhFileName, "folder/文件.txt", "文件夹/文件.txt"};
        for (const auto& file: fs::directory_iterator(outputPath)) {
            std::cout << "extracted file:" << file.path() << std::endl;
        }
        for (const auto& file: expected_files) { REQUIRE(fs::exists(outputPath + "/" + file)); }
    }
}
