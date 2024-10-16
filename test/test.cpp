#include "../SimZip.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporters_all.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {
    const std::string enFilename{"data.txt"};
    const std::string zhFileName{"中文文件.txt"};
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

    SECTION("create_zip3")
    {
        generateData(zhFileName);

        SimZip zip("test/test3.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(zhFileName) == true);
        REQUIRE(zip.add(zhFileName, "folder/文件.txt") == true);
        REQUIRE(zip.add(zhFileName, "文件夹/文件.txt") == true);
        REQUIRE(zip.add("不存在文件.txt") == false);
        zip.save();
    }

    SECTION("create_zip4")
    {
        generateData(u8"中文文件.txt");

        SimZip zip("test/test4.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(u8"中文文件.txt") == true);
        REQUIRE(zip.add(u8"中文文件.txt", u8"folder/文件.txt") == true);
        REQUIRE(zip.add(u8"中文文件.txt", u8"文件夹/文件.txt") == true);
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
    }

    SECTION("Extract all files from zip")
    {
        SimZip zip("test/test1.zip", SimZip::OpenMode::Read);
        zip.extractall(outputPath);
        std::vector<std::string> expected_files = {enFilename, "folder/rename.txt"};
        for (const auto& file: expected_files) { REQUIRE(fs::exists(outputPath + "/" + file)); }
    }

    SECTION("Extract single zh file from zip")
    {
        SimZip zip("test/test3.zip", SimZip::OpenMode::Read);
        zip.extract(zhFileName, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + zhFileName));
    }

    SECTION("Extract all zh files from zip")
    {
        SimZip zip("test/test3.zip", SimZip::OpenMode::Read);
        zip.extractall(outputPath);
        std::vector<std::string> expected_files = {zhFileName, "folder/文件.txt", "文件夹/文件.txt"};
        for (const auto& file: expected_files) { REQUIRE(fs::exists(outputPath + "/" + file)); }
    }

    SECTION("Extract all zh files from zip")
    {
        SimZip zip("test/test4.zip", SimZip::OpenMode::Read);
        zip.extractall(outputPath);
        std::vector<std::string> expected_files = {u8"中文文件.txt", u8"folder/文件.txt", u8"文件夹/文件.txt"};
        for (const auto& file: expected_files) { REQUIRE(fs::exists(outputPath + "/" + file)); }
    }
}
