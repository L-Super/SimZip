#include "SimZip.h"
#include "miniz.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <windows.h>

class SimZipPrivate {
public:
    explicit SimZipPrivate() = default;
    mz_zip_archive_file_stat readFileStat(int index);
    auto archiveFileIndex(const std::string& name);

private:
    friend SimZip;
    mz_zip_archive archive_{};
    std::string zipName_{};
    SimZip::OpenMode mode_{SimZip::OpenMode::None};
    mz_zip_archive_file_stat archiveFileStat_{};
};

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

SimZip::SimZip(const std::string& zipName, OpenMode mode) : d_ptr(new SimZipPrivate())
{
    d_ptr->zipName_ = zipName;
    d_ptr->mode_ = mode;

    switch (mode) {
        case OpenMode::Create: {
            if (!mz_zip_writer_init_file(&d_ptr->archive_, d_ptr->zipName_.c_str(), 0))
                throw std::runtime_error("Zip writer init failed");
        } break;
        case OpenMode::Read: {
            if (!mz_zip_reader_init_file(&d_ptr->archive_, d_ptr->zipName_.c_str(), 0)) {
                throw std::runtime_error("Failed opening zip archive " + d_ptr->zipName_);
            }
        } break;
        case OpenMode::None:
        default:
            break;
    }
}

SimZip::~SimZip()
{
    mz_zip_end(&d_ptr->archive_);
}

bool SimZip::add(const std::string& file, const std::string& archiveName)
{
    fs::path path(file);
    if (!fs::exists(path)) {
        std::cerr << "The file is not exist\n";
        return false;
    }
    if (d_ptr->mode_ != OpenMode::Create) {
        throw std::runtime_error("The current mode is not OpenMode::Create");
    }

    // Use the `filesystem` as an argument to resolve the issue of Chinese files not being able to be opened.
    std::ifstream ifs(path, std::ios::binary | std::ios::in);
    if (!ifs.is_open()) {
        std::cerr << file << " open failed\n";
        return false;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string bytes = ss.str();

    std::string archiveFileName;
    if (archiveName.empty()) {
        archiveFileName = path.filename().string();
    }
    else {
        archiveFileName = archiveName;
    }

    //    auto success = mz_zip_writer_add_file(&d_ptr->archive_, archiveFileName.c_str(),file.c_str(),"hello comment", 13, MZ_BEST_COMPRESSION);
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

    std::cout << "Created zip file " << d_ptr->zipName_ << ", file size: " << d_ptr->archive_.m_archive_size / 1024
              << "KB\n";
}

void SimZip::extract(const std::string& member, const std::string& path)
{
    int index = d_ptr->archiveFileIndex(member);

    fs::path dstPath(path);
    if (!is_directory(dstPath)) {
        throw std::logic_error("Extract path is not dir");
    }
    dstPath = fs::absolute(dstPath.append(member));

    if (!mz_zip_reader_extract_to_file(&d_ptr->archive_, index, dstPath.string().c_str(), 0)) {
        std::cerr << "Failed extracting " << member
                  << " from archive. Error string: " << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_))
                  << std::endl;
    }
}

void SimZip::extractall(const std::string& path)
{
    fs::path dstPath(path);
    if (!fs::exists(dstPath)) {
        fs::create_directory(dstPath);
    }

    auto nums = mz_zip_reader_get_num_files(&d_ptr->archive_);
    for (unsigned int i = 0; i < nums; i++) {
        mz_zip_archive_file_stat stat{};
        if (!mz_zip_reader_file_stat(&d_ptr->archive_, i, &stat)) {
            std::cerr << "Read archive file failed. Error string: "
                      << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_)) << std::endl;
        }
        bool isDir = stat.m_is_directory;
        std::cout << "stat: " << stat.m_filename << " index: " << stat.m_file_index
                  << " is dir: " << isDir << std::endl;
        // 文件名。如果字符串以 '/' 结尾，则为子目录条目。保证零端接，可截断以适合。
        std::string filename = stat.m_filename;
        std::cout << "filename: " << filename << std::endl;

        // TODO: 根据文件名是否有/，去做子目录下的文件处理
        if (mz_zip_reader_is_file_a_directory(&d_ptr->archive_, i)) {
            std::cout << "is dir" << std::endl;
            fs::path dir = dstPath / stat.m_filename;
            fs::create_directory(dir);
        }
        else {
            // TODO：solve GBK encoding
            try {
                fs::path filepath = fs::absolute(dstPath / stat.m_filename);
                if (!mz_zip_reader_extract_to_file(&d_ptr->archive_, stat.m_file_index, filepath.string().c_str(), 0)) {
                    std::cerr << "Failed extracting " << stat.m_filename << " from archive. Error string: "
                              << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_)) << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                std::string filepath = dstPath.string() + "/" + stat.m_filename;
                if (!mz_zip_reader_extract_to_file(&d_ptr->archive_, stat.m_file_index, filepath.c_str(), 0)) {
                    std::cerr << "Failed extracting " << stat.m_filename << " from archive. Error string: "
                              << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_)) << std::endl;
                }
            }
        }
    }
    close();
}

void SimZip::close()
{
    mz_zip_reader_end(&d_ptr->archive_);
}
