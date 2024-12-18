/*
MIT License

Copyright (c) 2024 L-Super

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "SimZip.h"
#include "miniz.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

#ifdef _MSC_VER
#include <windows.h>

// UTF-8 to wstring
std::wstring utf8_to_wstring(const std::string& s)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.size(), nullptr, 0);
    // the same as ws(len, '\0')
    std::wstring ws(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.size(), ws.data(), ws.size());
    return ws;
}

// wstring to UTF-8
std::string wstring_to_utf8(const std::wstring& ws)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size(), nullptr, 0, nullptr, nullptr);
    // the same as s(len, '\0')
    std::string s(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size(), s.data(), s.size(), nullptr, nullptr);
    return s;
}
#endif

class SimZipPrivate {
public:
    explicit SimZipPrivate() = default;
    void init(SimZip::OpenMode mode);
    mz_zip_archive_file_stat readFileStat(int index);
    auto archiveFileIndex(const std::string& name);

private:
    friend SimZip;
    mz_zip_archive archive_{};
    std::string zipName_{};
    SimZip::OpenMode mode_{SimZip::OpenMode::None};
};

void SimZipPrivate::init(SimZip::OpenMode mode)
{
    switch (mode) {
        case SimZip::OpenMode::Create: {
            if (!mz_zip_writer_init_file(&archive_, zipName_.c_str(), 0))
                throw std::runtime_error("Zip writer init failed");
        } break;
        case SimZip::OpenMode::Read: {
            if (!mz_zip_reader_init_file(&archive_, zipName_.c_str(), 0)) {
                throw std::runtime_error("Failed opening zip archive " + zipName_);
            }
        } break;
        case SimZip::OpenMode::None:
        default:
            break;
    }
}

mz_zip_archive_file_stat SimZipPrivate::readFileStat(int index)
{
    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(&archive_, index, &stat)) {
        std::cerr << "Failed read file index " << index
                  << " err: " << mz_zip_get_error_string(mz_zip_get_last_error(&archive_));
        return {};
    }
    return stat;
}

auto SimZipPrivate::archiveFileIndex(const std::string& name)
{
    int index = mz_zip_reader_locate_file(&archive_, name.c_str(), nullptr, 0);

    if (index < 0) {
        throw std::logic_error(name + " not found");
    }
    return index;
}

SimZip::SimZip(const std::string& zipName, OpenMode mode) : d_ptr(std::make_unique<SimZipPrivate>())
{
    d_ptr->zipName_ = zipName;
    d_ptr->mode_ = mode;
    d_ptr->init(mode);
}

SimZip::SimZip(const std::string& zipName) : d_ptr(std::make_unique<SimZipPrivate>())
{
    d_ptr->zipName_ = zipName;
}

SimZip::~SimZip()
{
    close();
}

void SimZip::setmode(SimZip::OpenMode mode)
{
    d_ptr->mode_ = mode;
    d_ptr->init(mode);
}

bool SimZip::add(const std::string& file, const std::string& archiveName)
{
#ifdef _MSC_VER
    fs::path filepath(utf8_to_wstring(file));
#else
    fs::path filepath(file);
#endif
    if (!fs::exists(filepath)) {
        std::cerr << "The file(" << file << ") is not exist\n";
        return false;
    }
    if (d_ptr->mode_ != OpenMode::Create) {
        throw std::runtime_error("The current mode is not OpenMode::Create");
    }

    // Use the `filesystem` as an argument to resolve the issue of Chinese files not being able to be opened.
    std::ifstream ifs(filepath, std::ios::binary | std::ios::in);
    if (!ifs.is_open()) {
        std::cerr << file << " open failed\n";
        return false;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string bytes = ss.str();
    ifs.close();

    std::string archiveFileName;
    if (archiveName.empty()) {
        archiveFileName = fs::path(file).filename().string();
    }
    else {
        archiveFileName = archiveName;
    }

    // auto success = mz_zip_writer_add_file(&d_ptr->archive_, archiveFileName.c_str(),file.c_str(),"hello comment", 13, MZ_BEST_COMPRESSION);
    auto success = mz_zip_writer_add_mem(&d_ptr->archive_, archiveFileName.c_str(), bytes.data(), bytes.size(),
                                         MZ_BEST_COMPRESSION);

    if (!success) {
        std::cerr << "Failed compress file: " << file << std::endl;
        return false;
    }

    return true;
}

void SimZip::save()
{
    if (d_ptr->mode_ != OpenMode::Create) {
        mz_zip_end(&d_ptr->archive_);
        return;
    }
    if (!mz_zip_writer_finalize_archive(&d_ptr->archive_)) {
        // if it was failed, then remove the zip file
        fs::remove(fs::path(d_ptr->zipName_));
        std::cerr << "Failed creating zip archive " << d_ptr->zipName_ << std::endl;
    }

    mz_zip_writer_end(&d_ptr->archive_);

    std::cout << "Created zip file " << d_ptr->zipName_ << ", file size: " << d_ptr->archive_.m_archive_size << "B\n";
}

bool SimZip::extract(const std::string& member, const std::string& path)
{
    int index = d_ptr->archiveFileIndex(member);

#ifdef _MSC_VER
    fs::path dir(utf8_to_wstring(path));
#else
    fs::path dir(path);
#endif
    if (!fs::exists(dir)) {
        // recursive create dir
        fs::create_directories(dir);
    }
    auto dstPath = path + "/" + member;

    // note: when the passed-in member contains a folder path, the extraction will fail.
    if (!mz_zip_reader_extract_to_file(&d_ptr->archive_, index, dstPath.c_str(), 0)) {
        std::cerr << "Failed extracting " << member
                  << " from archive. Error string: " << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_))
                  << std::endl;
        return false;
    }
    return true;
}

void SimZip::extractall(const std::string& path)
{
#ifdef _MSC_VER
    fs::path dstPath(utf8_to_wstring(path));
#else
    fs::path dstPath(path);
#endif
    if (!fs::exists(dstPath)) {
        // recursive create dir
        fs::create_directories(dstPath);
    }

    auto nums = mz_zip_reader_get_num_files(&d_ptr->archive_);
    for (unsigned int i = 0; i < nums; i++) {
        mz_zip_archive_file_stat stat{};
        if (!mz_zip_reader_file_stat(&d_ptr->archive_, i, &stat)) {
            std::cerr << "Read archive file failed. Error string: "
                      << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_)) << std::endl;
        }

        // create dir
        if (stat.m_is_directory) {
#ifdef _MSC_VER
            fs::path dir = dstPath / utf8_to_wstring(stat.m_filename);
#else
            fs::path dir = fs::absolute(dstPath / stat.m_filename);
#endif
            fs::create_directory(dir);
        }
        else {
#ifdef _MSC_VER
            fs::path filepath = fs::absolute(dstPath / utf8_to_wstring(stat.m_filename));
#else
            fs::path filepath = fs::absolute(dstPath / stat.m_filename);
#endif
            if (filepath.filename().string() != stat.m_filename && !fs::exists(filepath.parent_path())) {
                fs::create_directories(filepath.parent_path());
            }

            // UTF-8 string
            std::string outFilePath = path + "/" + stat.m_filename;
            if (!mz_zip_reader_extract_to_file(&d_ptr->archive_, stat.m_file_index, outFilePath.c_str(), 0)) {
                std::cerr << "Failed extracting " << stat.m_filename << " from archive. Error string: "
                          << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_)) << std::endl;
            }
        }
    }
    mz_zip_reader_end(&d_ptr->archive_);
}

void SimZip::close()
{
    mz_zip_end(&d_ptr->archive_);
}