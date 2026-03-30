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
        // 你好世界
        for (auto i = 0; i < 10; i++) { file << "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c \n"; }
        file.close();
    }
}

// 生成指定内容的UTF-8文件
void generateDataWithContent(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}

// 读取文件内容
std::string readFileContent(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    return content;
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

// ============================================================================
// Unicode / UTF-8 增强测试用例
// ============================================================================

TEST_CASE("UTF-8 content integrity", "[unicode][utf8][content]")
{
    fs::create_directory("test");

    SECTION("Chinese content preserved after compress and extract")
    {
        // 中文内容: 包含常用汉字、标点符号
        const std::string content = "\xe4\xb8\xad\xe6\x96\x87\xe6\xb5\x8b\xe8\xaf\x95\xef\xbc\x9a"
                                    "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c\xef\xbc\x81";
        // "中文测试：你好世界！"
        const std::string filename = "utf8_chinese.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_content_zh.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_content_zh_out";
        SimZip zip2("test/utf8_content_zh.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + filename));

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }

    SECTION("Japanese content preserved after compress and extract")
    {
        // 日本語: こんにちは世界
        const std::string content = "\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf"
                                    "\xe4\xb8\x96\xe7\x95\x8c";
        const std::string filename = "utf8_japanese.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_content_ja.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_content_ja_out";
        SimZip zip2("test/utf8_content_ja.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + filename));

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }

    SECTION("Korean content preserved after compress and extract")
    {
        // 한국어: 안녕하세요 세계
        const std::string content = "\xec\x95\x88\xeb\x85\x95\xed\x95\x98\xec\x84\xb8\xec\x9a\x94"
                                    " \xec\x84\xb8\xea\xb3\x84";
        const std::string filename = "utf8_korean.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_content_ko.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_content_ko_out";
        SimZip zip2("test/utf8_content_ko.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + filename));

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }

    SECTION("Emoji content preserved after compress and extract")
    {
        // 4字节UTF-8 Emoji: 😀🎉🚀🌍
        const std::string content = "\xf0\x9f\x98\x80\xf0\x9f\x8e\x89\xf0\x9f\x9a\x80\xf0\x9f\x8c\x8d";
        const std::string filename = "utf8_emoji.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_content_emoji.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_content_emoji_out";
        SimZip zip2("test/utf8_content_emoji.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + filename));

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }

    SECTION("Mixed multilingual content preserved")
    {
        // 混合多语言: English + 中文 + 日本語 + 한국어 + Emoji
        const std::string content = "Hello \xe4\xb8\x96\xe7\x95\x8c "
                                    "\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf "
                                    "\xec\x95\x88\xeb\x85\x95 "
                                    "\xf0\x9f\x98\x80\n";
        const std::string filename = "utf8_mixed.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_content_mixed.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_content_mixed_out";
        SimZip zip2("test/utf8_content_mixed.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + filename));

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }
}

TEST_CASE("Unicode filenames - multilingual", "[unicode][utf8][filename]")
{
    fs::create_directory("test");
    generateData(enFilename);

    SECTION("Japanese filename in archive")
    {
        // テスト.txt
        const std::string jaArchiveName = "\xe3\x83\x86\xe3\x82\xb9\xe3\x83\x88.txt";
        SimZip zip("test/test-ja-name.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename, jaArchiveName) == true);
        zip.save();

        const std::string outputPath = "test/output_ja_name";
        SimZip zip2("test/test-ja-name.zip", SimZip::OpenMode::Read);
        zip2.extract(jaArchiveName, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + jaArchiveName));
        fs::remove_all(outputPath);
    }

    SECTION("Korean filename in archive")
    {
        // 테스트.txt
        const std::string koArchiveName = "\xed\x85\x8c\xec\x8a\xa4\xed\x8a\xb8.txt";
        SimZip zip("test/test-ko-name.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename, koArchiveName) == true);
        zip.save();

        const std::string outputPath = "test/output_ko_name";
        SimZip zip2("test/test-ko-name.zip", SimZip::OpenMode::Read);
        zip2.extract(koArchiveName, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + koArchiveName));
        fs::remove_all(outputPath);
    }

    SECTION("Russian filename in archive")
    {
        // Тест.txt (Cyrillic)
        const std::string ruArchiveName = "\xd0\xa2\xd0\xb5\xd1\x81\xd1\x82.txt";
        SimZip zip("test/test-ru-name.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename, ruArchiveName) == true);
        zip.save();

        const std::string outputPath = "test/output_ru_name";
        SimZip zip2("test/test-ru-name.zip", SimZip::OpenMode::Read);
        zip2.extract(ruArchiveName, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + ruArchiveName));
        fs::remove_all(outputPath);
    }

    SECTION("Arabic filename in archive")
    {
        // اختبار.txt (Arabic)
        const std::string arArchiveName = "\xd8\xa7\xd8\xae\xd8\xaa\xd8\xa8\xd8\xa7\xd8\xb1.txt";
        SimZip zip("test/test-ar-name.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename, arArchiveName) == true);
        zip.save();

        const std::string outputPath = "test/output_ar_name";
        SimZip zip2("test/test-ar-name.zip", SimZip::OpenMode::Read);
        zip2.extract(arArchiveName, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + arArchiveName));
        fs::remove_all(outputPath);
    }
}

TEST_CASE("Unicode zip archive names", "[unicode][utf8][zipname]")
{
    fs::create_directory("test");
    generateData(enFilename);

    SECTION("Japanese zip archive name")
    {
        // アーカイブ.zip
        std::string zipName = "test/\xe3\x82\xa2\xe3\x83\xbc\xe3\x82\xab\xe3\x82\xa4\xe3\x83\x96.zip";
        SimZip zip(zipName, SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename) == true);
        zip.save();
        REQUIRE(fs::exists(zipName));

        SimZip zip2(zipName, SimZip::OpenMode::Read);
        const std::string outputPath = "test/output_ja_zip";
        zip2.extract(enFilename, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + enFilename));
        fs::remove_all(outputPath);
    }

    SECTION("Korean zip archive name")
    {
        // 아카이브.zip
        std::string zipName = "test/\xec\x95\x84\xec\xb9\xb4\xec\x9d\xb4\xeb\xb8\x8c.zip";
        SimZip zip(zipName, SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename) == true);
        zip.save();
        REQUIRE(fs::exists(zipName));

        SimZip zip2(zipName, SimZip::OpenMode::Read);
        const std::string outputPath = "test/output_ko_zip";
        zip2.extract(enFilename, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + enFilename));
        fs::remove_all(outputPath);
    }
}

TEST_CASE("Unicode folder paths in archive", "[unicode][utf8][folder]")
{
    fs::create_directory("test");
    generateData(enFilename);

    SECTION("Nested unicode folders")
    {
        // 第一层/第二层/data.txt
        const std::string archivePath =
            "\xe7\xac\xac\xe4\xb8\x80\xe5\xb1\x82/\xe7\xac\xac\xe4\xba\x8c\xe5\xb1\x82/data.txt";
        SimZip zip("test/test-nested-unicode-dir.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename, archivePath) == true);
        zip.save();

        const std::string outputPath = "test/output_nested_unicode";
        SimZip zip2("test/test-nested-unicode-dir.zip", SimZip::OpenMode::Read);
        zip2.extractall(outputPath);
        REQUIRE(fs::exists(outputPath + "/" + archivePath));
        fs::remove_all(outputPath);
    }

    SECTION("Mixed language folder paths")
    {
        // docs/文档/ドキュメント/file.txt
        const std::string archivePath =
            "docs/\xe6\x96\x87\xe6\xa1\xa3/"
            "\xe3\x83\x89\xe3\x82\xad\xe3\x83\xa5\xe3\x83\xa1\xe3\x83\xb3\xe3\x83\x88/file.txt";
        SimZip zip("test/test-mixed-lang-dir.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename, archivePath) == true);
        zip.save();

        const std::string outputPath = "test/output_mixed_lang_dir";
        SimZip zip2("test/test-mixed-lang-dir.zip", SimZip::OpenMode::Read);
        zip2.extractall(outputPath);
        REQUIRE(fs::exists(outputPath + "/" + archivePath));
        fs::remove_all(outputPath);
    }
}

TEST_CASE("UTF-8 special characters and edge cases", "[unicode][utf8][edge]")
{
    fs::create_directory("test");

    SECTION("UTF-8 BOM content")
    {
        // UTF-8 BOM: EF BB BF
        const std::string content = "\xef\xbb\xbf" "Hello with BOM\n";
        const std::string filename = "bom_test.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_bom.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_bom_out";
        SimZip zip2("test/utf8_bom.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }

    SECTION("Long Unicode filename in archive")
    {
        // 长中文文件名: 这是一个非常非常长的文件名用于测试.txt
        const std::string longName =
            "\xe8\xbf\x99\xe6\x98\xaf\xe4\xb8\x80\xe4\xb8\xaa"
            "\xe9\x9d\x9e\xe5\xb8\xb8\xe9\x9d\x9e\xe5\xb8\xb8"
            "\xe9\x95\xbf\xe7\x9a\x84\xe6\x96\x87\xe4\xbb\xb6"
            "\xe5\x90\x8d\xe7\x94\xa8\xe4\xba\x8e\xe6\xb5\x8b"
            "\xe8\xaf\x95.txt";
        generateData(enFilename);

        SimZip zip("test/test-long-unicode-name.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename, longName) == true);
        zip.save();

        const std::string outputPath = "test/output_long_unicode";
        SimZip zip2("test/test-long-unicode-name.zip", SimZip::OpenMode::Read);
        zip2.extract(longName, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + longName));
        fs::remove_all(outputPath);
    }

    SECTION("Special Unicode symbols in content")
    {
        // 特殊符号: ©®™€£¥°±×÷
        const std::string content = "\xc2\xa9\xc2\xae\xe2\x84\xa2"
                                    "\xe2\x82\xac\xc2\xa3\xc2\xa5"
                                    "\xc2\xb0\xc2\xb1\xc3\x97\xc3\xb7\n";
        const std::string filename = "utf8_symbols.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_symbols.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_symbols_out";
        SimZip zip2("test/utf8_symbols.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }

    SECTION("CJK Unified Ideographs Extension B (4-byte UTF-8)")
    {
        // 𠀀 (U+20000) - CJK Extension B character, 4-byte UTF-8: F0 A0 80 80
        const std::string content = "\xf0\xa0\x80\x80\xf0\xa0\x80\x81\xf0\xa0\x80\x82\n";
        const std::string filename = "utf8_cjk_ext_b.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_cjk_ext_b.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_cjk_ext_b_out";
        SimZip zip2("test/utf8_cjk_ext_b.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }
}

TEST_CASE("Multiple unicode files in one archive", "[unicode][utf8][multiple]")
{
    fs::create_directory("test");

    // 准备多个不同语言的文件
    const std::string zhContent = "\xe4\xb8\xad\xe6\x96\x87\xe5\x86\x85\xe5\xae\xb9\n";  // 中文内容
    const std::string jaContent = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e\xe5\x86\x85\xe5\xae\xb9\n";  // 日本語内容
    const std::string koContent = "\xed\x95\x9c\xea\xb5\xad\xec\x96\xb4\xeb\x82\xb4\xec\x9a\xa9\n";  // 한국어내용
    const std::string mixContent = "English + \xe4\xb8\xad\xe6\x96\x87 + \xf0\x9f\x98\x80\n";

    const std::string zhFile = "\xe4\xb8\xad\xe6\x96\x87.txt";      // 中文.txt
    const std::string jaFile = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e.txt";  // 日本語.txt
    const std::string koFile = "\xed\x95\x9c\xea\xb5\xad\xec\x96\xb4.txt";  // 한국어.txt
    const std::string mixFile = "mixed_lang.txt";

    generateDataWithContent(zhFile, zhContent);
    generateDataWithContent(jaFile, jaContent);
    generateDataWithContent(koFile, koContent);
    generateDataWithContent(mixFile, mixContent);

    SECTION("Create archive with multiple unicode-named files")
    {
        SimZip zip("test/multi-unicode.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(zhFile) == true);
        REQUIRE(zip.add(jaFile) == true);
        REQUIRE(zip.add(koFile) == true);
        REQUIRE(zip.add(mixFile) == true);
        zip.save();
        REQUIRE(fs::exists("test/multi-unicode.zip"));
    }

    SECTION("Extract all unicode-named files and verify content")
    {
        // 先创建压缩包
        {
            SimZip zip("test/multi-unicode-verify.zip", SimZip::OpenMode::Create);
            zip.add(zhFile);
            zip.add(jaFile);
            zip.add(koFile);
            zip.add(mixFile);
            zip.save();
        }

        const std::string outputPath = "test/output_multi_unicode";
        SimZip zip2("test/multi-unicode-verify.zip", SimZip::OpenMode::Read);
        zip2.extractall(outputPath);

        // 验证每个文件存在且内容正确
        REQUIRE(fs::exists(outputPath + "/" + zhFile));
        REQUIRE(fs::exists(outputPath + "/" + jaFile));
        REQUIRE(fs::exists(outputPath + "/" + koFile));
        REQUIRE(fs::exists(outputPath + "/" + mixFile));

        REQUIRE(readFileContent(outputPath + "/" + zhFile) == zhContent);
        REQUIRE(readFileContent(outputPath + "/" + jaFile) == jaContent);
        REQUIRE(readFileContent(outputPath + "/" + koFile) == koContent);
        REQUIRE(readFileContent(outputPath + "/" + mixFile) == mixContent);

        fs::remove_all(outputPath);
    }

    // 清理临时文件
    fs::remove(zhFile);
    fs::remove(jaFile);
    fs::remove(koFile);
    fs::remove(mixFile);
}

TEST_CASE("Unicode filename with rename in archive", "[unicode][utf8][rename]")
{
    fs::create_directory("test");

    SECTION("Rename ASCII file to Unicode name")
    {
        generateData(enFilename);
        // 重命名为: 数据文件.txt
        const std::string unicodeName = "\xe6\x95\xb0\xe6\x8d\xae\xe6\x96\x87\xe4\xbb\xb6.txt";

        SimZip zip("test/test-rename-to-unicode.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(enFilename, unicodeName) == true);
        zip.save();

        const std::string outputPath = "test/output_rename_unicode";
        SimZip zip2("test/test-rename-to-unicode.zip", SimZip::OpenMode::Read);
        zip2.extract(unicodeName, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + unicodeName));
        fs::remove_all(outputPath);
    }

    SECTION("Rename Unicode file to ASCII name")
    {
        generateData(zhFileName);

        SimZip zip("test/test-rename-to-ascii.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(zhFileName, "renamed_data.txt") == true);
        zip.save();

        const std::string outputPath = "test/output_rename_ascii";
        SimZip zip2("test/test-rename-to-ascii.zip", SimZip::OpenMode::Read);
        zip2.extract("renamed_data.txt", outputPath);
        REQUIRE(fs::exists(outputPath + "/renamed_data.txt"));
        fs::remove_all(outputPath);
    }

    SECTION("Rename Unicode file to different Unicode name")
    {
        generateData(zhFileName);
        // 重命名为日文: データ.txt
        const std::string jaName = "\xe3\x83\x87\xe3\x83\xbc\xe3\x82\xbf.txt";

        SimZip zip("test/test-rename-unicode-to-unicode.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(zhFileName, jaName) == true);
        zip.save();

        const std::string outputPath = "test/output_rename_u2u";
        SimZip zip2("test/test-rename-unicode-to-unicode.zip", SimZip::OpenMode::Read);
        zip2.extract(jaName, outputPath);
        REQUIRE(fs::exists(outputPath + "/" + jaName));
        fs::remove_all(outputPath);
    }
}

TEST_CASE("Empty and whitespace Unicode content", "[unicode][utf8][edge]")
{
    fs::create_directory("test");

    SECTION("File with only Unicode whitespace characters")
    {
        // 全角空格 (U+3000) + 普通空格
        const std::string content = "\xe3\x80\x80 \xe3\x80\x80 \xe3\x80\x80\n";
        const std::string filename = "utf8_whitespace.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_whitespace.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_whitespace_out";
        SimZip zip2("test/utf8_whitespace.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }

    SECTION("File with zero-width characters in content")
    {
        // Zero-width space (U+200B) + Zero-width non-joiner (U+200C)
        const std::string content = "A\xe2\x80\x8b\xe2\x80\x8C\n";
        const std::string filename = "utf8_zerowidth.txt";
        generateDataWithContent(filename, content);

        SimZip zip("test/utf8_zerowidth.zip", SimZip::OpenMode::Create);
        REQUIRE(zip.add(filename) == true);
        zip.save();

        const std::string outputPath = "test/utf8_zerowidth_out";
        SimZip zip2("test/utf8_zerowidth.zip", SimZip::OpenMode::Read);
        zip2.extract(filename, outputPath);

        std::string extracted = readFileContent(outputPath + "/" + filename);
        REQUIRE(extracted == content);

        fs::remove(filename);
        fs::remove_all(outputPath);
    }
}
