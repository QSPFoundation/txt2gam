# TXT2GAM

## Linux build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Windows build

```bash
mkdir build
cd build
cmake -G "Visual Studio 15 2017" -A Win32 -DCMAKE_INSTALL_PREFIX=out ..
cmake --build . --target install --config Release
```

## TODO

* Support UTF-8
* Support all new-line formats (CRLF, CR, LF)
* Extend TXT2GAM format to support optional base description & base actions
* Join multiple text files (you can specify a file that contains a list of files to be joined)

## Chat group

https://discord.gg/6gWVYUtUGZ
