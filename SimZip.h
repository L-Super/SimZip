//
// Created by LMR on 24-5-9.
//

#pragma once

#include <string>
#include <memory>
#include <vector>

class SimZipPrivate;
class SimZip {
public:
    SimZip();
    ~SimZip();

    void write(const std::string& filename);
    void write(const std::string &filename, const std::string &archiveName);
    void write(const std::vector<std::string>& files);

    void save(const std::string&  filename);

private:
    std::unique_ptr<SimZipPrivate> d_ptr;
};