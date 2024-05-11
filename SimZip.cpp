#include <iostream>
#include <fstream>
#include <sstream>
#include "SimZip.h"
#include "miniz.h"

class SimZipPrivate {
public:
    explicit SimZipPrivate() = default;

private:
    friend SimZip;
    mz_zip_archive archive_{};
    std::string archiveName;
};

SimZip::SimZip(const std::string &zipName) : d_ptr(new SimZipPrivate()) {
    d_ptr->archiveName = zipName;
    if (!mz_zip_writer_init_file(&d_ptr->archive_, d_ptr->archiveName.c_str(), 0)) {
        std::cerr << "zip writer init failed\n";
    }
}

SimZip::~SimZip() {

}

bool SimZip::add(const std::string &file, const std::string &archiveName) {
    std::ifstream ifs(file, std::ios::binary | std::ios::in);
    if (!ifs.is_open()) {
        std::cerr << file << " open failed\n";
        return false;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string bytes = ss.str();

    std::string archiveFileName;
    if (archiveName.empty()) {
        fs::path path(file);
        archiveFileName = path.filename().string();
    }
    else{
        archiveFileName = archiveName;
    }

    auto success = mz_zip_writer_add_mem(&d_ptr->archive_, archiveFileName.c_str(), bytes.data(),
                                         bytes.size(),
                                         MZ_BEST_COMPRESSION);

    if (!success) {
        std::cerr << "Failed compress file: " << file << std::endl;
        return false;
    }

    return true;
}

void SimZip::save() {
    if (!mz_zip_writer_finalize_archive(&d_ptr->archive_)) {
        // if it was failed, then remove the zip file
        fs::remove(fs::path(d_ptr->archiveName));
        std::cerr << "Failed creating zip archive " << d_ptr->archiveName << std::endl;
    }

    mz_zip_writer_end(&d_ptr->archive_);

    std::cout << "Created zip file " << d_ptr->archiveName << ", file size: " << d_ptr->archive_.m_archive_size / 1024
              << "KB\n";
}




