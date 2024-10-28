#include "../SimZip.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporters_all.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace {
    const std::string enFilename{"data.txt"};
    const std::string zhFileName{"文件.txt"};
}// namespace

void generateData(const std::string& filename)
{
    std::ofstream file(filename);
    if (file.is_open()) {
        for (auto i = 0; i < 10; i++) { file << "this is data for test.\n"; }
        file.close();
    }
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
CATCH_REGISTER_LISTENER(CleanupListener)

TEST_CASE("create zip", "[create_zip]")
{
    fs::create_directory("test");
    generateData(enFilename);
    SECTION("create_zip1")
    {
        SimZip zip("test/test1.zip", SimZip::OpenMode::Create);
        auto result = zip.add(enFilename);
        REQUIRE(result.ok == true);
        REQUIRE(result.latestError.empty() == true);
        result = zip.add(enFilename, "folder/rename.txt");
        REQUIRE(result.ok == true);
        REQUIRE(result.latestError.empty() == true);
        result = zip.add("empty.txt");
        REQUIRE(result.ok == false);
        REQUIRE(result.latestError.empty() == false);
        zip.save();
    }

    SECTION("create_zip2")
    {
        SimZip zip("test/test1.zip");
        zip.setmode(SimZip::OpenMode::Create);
        auto result = zip.add(enFilename);
        REQUIRE(result.ok == true);
        result = zip.add(enFilename, "folder/rename.txt");
        REQUIRE(result.ok == true);
        result = zip.add("empty.txt");
        REQUIRE(result.ok == false);
        REQUIRE(result.latestError.empty() == false);
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
        auto result = zip.add(enFilename);
        REQUIRE(result.ok == true);
        result = zip.add(enFilename, "folder/rename.txt");
        REQUIRE(result.ok == true);
        result = zip.add("empty.txt");
        REQUIRE(result.ok == false);
        REQUIRE(result.latestError.empty() == false);
        zip.save();
        REQUIRE(fs::exists(zipName));
    }

    SECTION("create_unicode_zip2")
    {
        generateData(zhFileName);
        SimZip zip("test/test-unicode.zip", SimZip::OpenMode::Create);
        auto result = zip.add(zhFileName);
        REQUIRE(result.ok == true);
        result = zip.add(zhFileName, "folder/文件.txt");
        REQUIRE(result.ok == true);
        result = zip.add(zhFileName, "文件夹/文件.txt");
        REQUIRE(result.ok == true);
        result = zip.add("不存在文件.txt");
        REQUIRE(result.ok == false);
        REQUIRE(result.latestError.empty() == false);
        zip.save();
    }
}

TEST_CASE("extract zip", "[extract_zip]")
{
    const std::string outputPath{"test/output"};
    SECTION("Extract single file from zip")
    {
        SimZip zip("test/test1.zip", SimZip::OpenMode::Read);
        auto result = zip.extract(enFilename, outputPath);
        REQUIRE(result.ok == true);
        REQUIRE(result.latestError.empty() == true);
        REQUIRE(fs::exists(outputPath + "/" + enFilename));
        fs::remove_all(outputPath);
    }

    SECTION("Extract all files from zip")
    {
        SimZip zip("test/test1.zip", SimZip::OpenMode::Read);
        auto result = zip.extractall(outputPath);
        REQUIRE(result.ok == true);
        REQUIRE(result.latestError.empty() == true);
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
        auto result = zip.extract(enFilename,outputPath);
        REQUIRE(result.ok == true);
        for (const auto& file: fs::directory_iterator(outputPath)) {
            std::cout << "extracted file:" << file.path() << std::endl;
        }
        REQUIRE(fs::exists(outputPath + "/" + enFilename));
    }

    SECTION("Extract single unicode file from zip")
    {
        const std::string outputPath{"test/output2"};
        SimZip zip("test/test-unicode.zip", SimZip::OpenMode::Read);
        auto result = zip.extract(zhFileName, outputPath);
        REQUIRE(result.ok == true);
        for (const auto& file: fs::directory_iterator(outputPath)) {
            std::cout << "extracted file:" << file.path() << std::endl;
        }
        REQUIRE(fs::exists(outputPath + "/" + zhFileName));
    }

    SECTION("Extract all unicode files from zip")
    {
        const std::string outputPath{"test/output3"};
        SimZip zip("test/test-unicode.zip", SimZip::OpenMode::Read);
        auto result = zip.extractall(outputPath);
        REQUIRE(result.ok == true);
        std::vector<std::string> expected_files = {zhFileName, "folder/文件.txt", "文件夹/文件.txt"};
        for (const auto& file: fs::directory_iterator(outputPath)) {
            std::cout << "extracted file:" << file.path() << std::endl;
        }
        for (const auto& file: expected_files) { REQUIRE(fs::exists(outputPath + "/" + file)); }
    }
}
