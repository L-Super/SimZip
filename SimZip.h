//
// Created by LMR on 24-5-9.
//

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

//TODO:
// + support read file info

class SimZipPrivate;
class SimZip {
public:
    enum class OpenMode{
        Read,
        Create,
        None
    };
public:
    explicit SimZip(const std::string & zipName,  OpenMode mode);
    ~SimZip();

    bool add(const std::string &file, const std::string &archiveName = {});

    void save();

    void extract(const std::string &member, const std::string &path);
    void extractall(const std::string &path);

    void close();

private:
    std::unique_ptr<SimZipPrivate> d_ptr;
};