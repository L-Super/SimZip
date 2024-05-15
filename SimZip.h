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

#pragma once

#include <string>
#include <memory>
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