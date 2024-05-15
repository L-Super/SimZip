#include "../SimZip.h"
#include <filesystem>

// create zip
void createZip()
{
    SimZip zip("test.zip", SimZip::OpenMode::Create);
    zip.add("data.txt");
    zip.add("data.txt", "folder/rename.txt");
    zip.save();
}

// extract zip
void extractZip()
{
    SimZip zip("test.zip", SimZip::OpenMode::Read);
    zip.extract("data.txt", "output/");
    zip.extractall("output/");
}

void clear()
{
    fs::remove("data.txt");
    fs::remove("test.zip");
    fs::remove_all("output");
}

int main()
{
    createZip();
    extractZip();
    clear();

    return 0;
}
