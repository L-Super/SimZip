#include "SimZip.h"

int main() {
    // create zip
//    SimZip zip("test.zip", SimZip::OpenMode::Create);
//    zip.add("p.pdf");
////    zip.add("计算机图形学.pdf");
//    zip.add("新建文本.txt");
//    zip.add("../SimZip.cpp", "src/Sim.cpp");
//    zip.add("sky.jpg");
//    zip.save();


//    try {
        SimZip zip("test1.zip", SimZip::OpenMode::Read);
//        zip.extract("p.pdf", "../");
//        zip.extract("sky.jpg", "../");
//        zip.extract("新建文本.txt", "../");
        //    zip.save();
        zip.extractall("ex/");
//    }
//    catch (const std::runtime_error& e) {
//        std::cerr << "运行时错误: " << e.what() << std::endl;
//    }

    return 0;
}
