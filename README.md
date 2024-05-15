## SimZip - A C++ Wrapper for miniz Library
SimZip is a lightweight C++ 17 library that provides easy-to-use functionalities for zip and unzip operations using the `miniz` library. 

It aims to simplify the process of working with ZIP archives in C++ projects.

## Features
- Simple and intuitive API for compressing and decompressing files.
- Cross-platform (Windows, Linux, macOS).
- No external dependencies other than the miniz library.

## Usage
Here's a basic example of how to use SimZip to compress and decompress files:

```cpp
#include "SimZip.h"
// create a zip
SimZip zip("test.zip", SimZip::OpenMode::Create);
zip.add("test.txt"); // add file
zip.add("../SimZip.cpp", "folder/SimZip.cpp"); // add file to folder
zip.add("test1.txt", "text.txt"); // add and rename file
zip.save();

// extract zip
SimZip zip("test.zip", SimZip::OpenMode::Read);
zip.extract("test.txt", "output/"); //extract single file
zip.extractall("output/"); // extract all files
```

## Contributing
Contributions are welcome!

## License
SimZip is distributed under the [MIT](LICENSE) License.
