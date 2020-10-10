@echo off

cd "C:\Users\alexe\workspace\sptk5"
git reset --hard
git pull

cd "C:\Users\alexe\workspace\sptk5\build" 2>&1 > build.log

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" 2>&1 >> build.log
if errorlevel 1 (
    type build.log
    exit /b %errorlevel%
)

rmdir /S /Q build64 2>&1 >> build.log

mkdir build64 2>&1 >> build.log
cd build64
if errorlevel 1 (
    echo "Can't switch to build directory"
    exit /b %errorlevel%
)

cmake -G "Visual Studio 16 2019" -A x64 -DUSE_GTEST=OFF -DBUILD_EXAMPLES=OFF "C:\Users\alexe\workspace\sptk5\code"
if errorlevel 1 (
    echo "Can't configure build"
    exit /b %errorlevel%
)
cd ..

cmake --build build64 --config Release --target INSTALL
if errorlevel 1 (
    echo "Can't complete build and install"
    exit /b %errorlevel%
)

"C:\Program Files (x86)\Caphyon\Advanced Installer 16.5\bin\x86\advinst.exe" /build SPTK.aip 2>&1 >> build.log
if errorlevel 1 (
    echo "Can't build installer"
    exit /b %errorlevel%
)

mkdir Downloads 2>&1 >> build.log

set /p VERSION=<VERSION

mv SPTK-SetupFiles\SPTK.exe Downloads\SPTK-%VERSION%.exe 2>&1 >> build.log
if errorlevel 1 (
    echo "Can't move installer to Downloads directory"
    exit /b %errorlevel%
)

rmdir /S /Q SPTK-SetupFiles SPTK-cache 2>&1 >> build.log

scp -i ssh\id_rsa -P 444 Downloads\SPTK-%VERSION%.exe alexeyp@www.sptk.net:/var/www/sites/sptk/download/%VERSION%/SPTK-%VERSION%.exe
