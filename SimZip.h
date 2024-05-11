//
// Created by LMR on 24-5-9.
//

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>

#include <cstdarg>
#include "miniz.h"

namespace fs = std::filesystem;

class SimZipPrivate;
class SimZip {
public:
    explicit SimZip(const std::string & zipName);
    ~SimZip();

    bool add(const std::string & file);
    bool add(const std::string &file, const std::string &archiveName);

    void save();

private:
    std::unique_ptr<SimZipPrivate> d_ptr;
};

static void print_error(const char *pMsg, ...)
{
    char buf[1024];

    va_list args;
    va_start(args, pMsg);
    vsnprintf(buf, sizeof(buf), pMsg, args);
    va_end(args);

    buf[sizeof(buf) - 1] = '\0';

    fprintf(stderr, "Error: %s", buf);
}
static bool zip_create(const char *pZip_filename, const char *pSrc_filename)
{
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if (!mz_zip_writer_init_file(&zip, pZip_filename, 65537))
    {
        print_error("Failed creating zip archive \"%s\" (1)!\n", pZip_filename);
        return false;
    }

    mz_bool success = MZ_TRUE;

    const char *pStr = "This is a test!This is a test!This is a test!\n";

    success &= mz_zip_writer_add_mem(&zip, "cool/", NULL, 0, 0);

    success &= mz_zip_writer_add_mem(&zip, "1.txt", pStr, strlen(pStr), 9);

    for (int i = 0; i < 2; i++)
    {
        char name[256], buf[256], comment[256];
        sprintf(name, "t%u.txt", i);
        sprintf(buf, "%u\n", i*5377);
        sprintf(comment, "comment: %u\n", i);
        success &= mz_zip_writer_add_mem_ex(&zip, name, buf, strlen(buf), comment, (unsigned short)strlen(comment), i % 10, 0, 0);
    }

    const char *pTestComment = "test comment";
    success &= mz_zip_writer_add_file(&zip, "test.bin", pSrc_filename, pTestComment, (unsigned short)strlen(pTestComment), 9);

    if (!success)
    {
        mz_zip_writer_end(&zip);
        remove(pZip_filename);
        print_error("Failed creating zip archive \"%s\" (2)!\n", pZip_filename);
        return false;
    }

    if (!mz_zip_writer_finalize_archive(&zip))
    {
        mz_zip_writer_end(&zip);
        remove(pZip_filename);
        print_error("Failed creating zip archive \"%s\" (3)!\n", pZip_filename);
        return false;
    }

    mz_zip_writer_end(&zip);

    printf("Created zip file \"%s\", file size:  %llu \n", pZip_filename, zip.m_archive_size);
    return true;
}