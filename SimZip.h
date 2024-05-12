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

//TODO:
// + support dir
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

static size_t zip_write_callback(void* pOpaque, mz_uint64 ofs, const void* pBuf, size_t n)
{
    (void) pOpaque, (void) ofs, (void) pBuf, (void) n;
    return n;
}
static bool zip_extract(const char* pZip_filename, const char* pDst_filename)
{
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, pZip_filename, 0)) {
        print_error("Failed opening zip archive \"%s\"!\n", pZip_filename);
        return false;
    }

    int file_index = mz_zip_reader_locate_file(&zip, "test.bin", "test Comment", 0);
    int alt_file_index = mz_zip_reader_locate_file(&zip, "test.bin", "test Comment e", 0);
    if ((file_index < 0) || (alt_file_index >= 0)) {
        print_error("Archive \"%s\" is missing test.bin file!\n", pZip_filename);
        mz_zip_reader_end(&zip);
        return false;
    }

    alt_file_index = mz_zip_reader_locate_file(&zip, "test.bin", NULL, 0);
    if (alt_file_index != file_index) {
        print_error("mz_zip_reader_locate_file() failed!\n", pZip_filename);
        mz_zip_reader_end(&zip);
        return false;
    }

    if (!mz_zip_reader_extract_to_file(&zip, file_index, pDst_filename, 0)) {
        print_error("Failed extracting test.bin from archive \"%s\" err: %s!\n", pZip_filename,
                    mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
        mz_zip_reader_end(&zip);
        return false;
    }

    for (unsigned int i = 0; i < mz_zip_reader_get_num_files(&zip); i++) {
        mz_zip_archive_file_stat stat;
        if (!mz_zip_reader_file_stat(&zip, i, &stat)) {
            print_error("Failed testing archive -1 \"%s\" err: %s!\n", pZip_filename,
                        mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
            mz_zip_reader_end(&zip);
            return false;
        }
        //printf("\"%s\" %I64u %I64u\n", stat.m_filename, stat.m_comp_size, stat.m_uncomp_size);
        size_t size = 0;

        mz_bool status = mz_zip_reader_extract_to_callback(&zip, i, zip_write_callback, NULL, 0);
        if (!status) {
            print_error("Failed testing archive -2 \"%s\" err: %s!\n", pZip_filename,
                        mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
            mz_zip_reader_end(&zip);
            return false;
        }

        if (stat.m_uncomp_size < 100 * 1024 * 1024) {
            void* p = mz_zip_reader_extract_to_heap(&zip, i, &size, 0);
            if (!p) {
                print_error("Failed testing archive -3 \"%s\" err: %s!\n", pZip_filename,
                            mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
                mz_zip_reader_end(&zip);
                return false;
            }
            free(p);
        }

        if (stat.m_uncomp_size < 100 * 1024 * 1024) {
            /* Use iterative reader to read onto heap in one big chunk */
            mz_zip_reader_extract_iter_state* pIter = mz_zip_reader_extract_iter_new(&zip, i, 0);
            void* p = malloc(stat.m_uncomp_size);
            if ((!pIter) && (0 != stat.m_uncomp_size)) {
                print_error("Failed testing archive -4 \"%s\" err: %s!\n", pZip_filename,
                            mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
                free(p);
                mz_zip_reader_end(&zip);
                return false;
            }
            if (pIter) {
                if (stat.m_uncomp_size != mz_zip_reader_extract_iter_read(pIter, p, stat.m_uncomp_size)) {
                    print_error("Failed testing archive -5 \"%s\" err: %s!\n", pZip_filename,
                                mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
                    free(p);
                    mz_zip_reader_extract_iter_free(pIter);
                    mz_zip_reader_end(&zip);
                    return false;
                }
                if (MZ_TRUE != mz_zip_reader_extract_iter_free(pIter)) {
                    print_error("Failed testing archive -6 \"%s\" err: %s!\n", pZip_filename,
                                mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
                    free(p);
                    mz_zip_reader_end(&zip);
                    return false;
                }
            }
            free(p);
        }

        if (stat.m_uncomp_size < 100 * 1024) {
            /* Use iterative reader to read file one byte at a time */
            mz_zip_reader_extract_iter_state* pIter = mz_zip_reader_extract_iter_new(&zip, i, 0);
            uint8_t byBuffer;
            if ((!pIter) && (0 != stat.m_uncomp_size)) {
                print_error("Failed testing archive -7 \"%s\" err: %s!\n", pZip_filename,
                            mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
                mz_zip_reader_end(&zip);
                return false;
            }
            if (pIter) {
                for (uint64_t uiIndex = 0; uiIndex < stat.m_uncomp_size; uiIndex++) {
                    if (sizeof(byBuffer) != mz_zip_reader_extract_iter_read(pIter, &byBuffer, sizeof(byBuffer))) {
                        print_error("Failed testing archive -8 \"%s\" err: %s!\n", pZip_filename,
                                    mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
                        mz_zip_reader_extract_iter_free(pIter);
                        mz_zip_reader_end(&zip);
                        return false;
                    }
                }
                if (MZ_TRUE != mz_zip_reader_extract_iter_free(pIter)) {
                    print_error("Failed testing archive -9 \"%s\" err: %s!\n", pZip_filename,
                                mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
                    mz_zip_reader_end(&zip);
                    return false;
                }
            }
        }
    }
    printf("Verified %u files\n", mz_zip_reader_get_num_files(&zip));

    mz_zip_reader_end(&zip);

    printf("Extracted file \"%s\"\n", pDst_filename);
    return true;
}