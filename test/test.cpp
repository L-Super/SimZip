#include "../SimZip.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

void generateData()
{
    std::ofstream file("data.txt");
    if (file.is_open()) {
        for (auto i = 0; i < 10; i++) { file << "this is data for test.\n"; }
        file.close();
    }
}

// create zip
void createZip()
{
    SimZip zip("test.zip", SimZip::OpenMode::Create);
    zip.add("data.txt");
    zip.add("data.txt", "folder/rename.txt");
    zip.save();
}

// extract zip
void extractZip()
{
    SimZip zip("test.zip", SimZip::OpenMode::Read);
    zip.extract("data.txt", "output/");
    zip.extractall("output/");
}

void clear()
{
    fs::remove("data.txt");
    fs::remove("test.zip");
    fs::remove_all("output");
}

TEST_CASE("create zip", "[create_zip]")
{
    generateData();
    SimZip zip("test.zip", SimZip::OpenMode::Create);
    REQUIRE(zip.add("data.txt") == true);
    REQUIRE(zip.add("data.txt", "folder/rename.txt") == true);
    REQUIRE(zip.add("empty.txt") == false);
    zip.save();
}

TEST_CASE("extract zip", "[extract_zip]")
{
    SimZip zip("test.zip", SimZip::OpenMode::Read);

    SECTION("Extract single file from zip")
    {
        zip.extract("data.txt", "output/");
        REQUIRE(fs::exists("output/data.txt"));
    }
    SECTION("Extract all files from zip")
    {
        zip.extractall("output/");
        std::vector<std::string> expected_files = {"data.txt", "folder/rename.txt"};
        for (const auto& file: expected_files) { REQUIRE(fs::exists("output/" + file)); }
    }
}

TEST_CASE("clean job")
{
    clear();
}
