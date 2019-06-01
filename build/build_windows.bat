cd "C:\Users\alexe\workspace\sptk5\build\test"

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

mkdir build64
rem del /S /Q build64\*.*
cd build64

cmake -G "Visual Studio 16 2019" -A x64 -DUSE_GTEST=OFF "C:\Users\alexe\workspace\sptk5\code"
cd ..

cmake --build build64 --config Release --target INSTALL

REM devenv ..\code\w64\SPTK.sln /build RelWithDebInfo /project INSTALL
