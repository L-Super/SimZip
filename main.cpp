#include <iostream>
#include "SimZip.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
//    zip_create("hh.zip", "F:/Code/CppProjects/SimZip/README.md");

    SimZip zip("test.zip");
//    zip.add("F:/Code/CppProjects/SimZip/");
    zip.add("F:/Code/CppProjects/SimZip/README.md");
    zip.add("F:/Code/CppProjects/SimZip/SimZip.cpp");
    zip.save();
    return 0;
}
