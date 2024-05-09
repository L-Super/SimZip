#include "SimZip.h"
#include "miniz.h"

class SimZipPrivate {
public:
    explicit SimZipPrivate() {}


private:
    mz_zip_archive* archive_;
};

SimZip::SimZip():d_ptr(new SimZipPrivate()) {

}

SimZip::~SimZip() {

}

void SimZip::write(const std::string &filename) {

}

void SimZip::write(const std::string &filename, const std::string &archiveName) {

}

void SimZip::write(const std::vector<std::string> &files) {

}

void SimZip::save(const std::string &filename) {

}
