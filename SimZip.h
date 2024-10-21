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

#include <memory>
#include <string>

class SimZipPrivate;
class SimZip {
public:
    /**
     * Enum class defining the modes in which a ZIP file can be opened
     */
    enum class OpenMode {
        Read,  ///< Mode for reading an existing ZIP file.
        Create,///< Mode for creating a new ZIP file.
        None,  ///< Default mode.
    };

public:
    /**
     * SimZip constructor
     * @param zipName The name of the zip file that needs to be created or opened.
     * @param mode The mode in which the ZIP file is opened.
     */
    explicit SimZip(const std::string& zipName, OpenMode mode);
    /**
     * SimZip constructor
     * @param zipName The name of the zip file that needs to be created or opened.
     */
    explicit SimZip(const std::string& zipName);
    ~SimZip();

    /**
     * Sets the mode for the ZIP archive operation
     * This function allows the user to change the mode of the ZIP archive after the object has been created.
     * @param mode The mode to set for the ZIP archive operation.
     * The mode determines the behavior of subsequent operations on the archive.
     * @note It is recommended to set the mode at object construction and not change it mid-operation.
     */
    void setmode(OpenMode mode);

    /**
     * Adds a file to the ZIP archive
     * @param file The path of the file to be added.
     * @param archiveName The internal path within the ZIP file (optional, defaults to the original file path).
     * @return  true if successful, false otherwise.
     */
    bool add(const std::string& file, const std::string& archiveName = {});

    /**
     * Saves the ZIP file to disk
     */
    void save();

    /**
     * Extracts a single file from the ZIP archive
     * @param member The internal path of the file within the ZIP archive.
     * @param path The path to which the file will be extracted.
     * @return true if successful, false otherwise.
     */
    bool extract(const std::string& member, const std::string& path);
    /**
     * Extracts all files from the ZIP archive
     * @param path The path to which all files will be extracted.
     */
    void extractall(const std::string& path);

    /**
     * close zip handle
     */
    void close();

private:
    std::unique_ptr<SimZipPrivate> d_ptr;
};