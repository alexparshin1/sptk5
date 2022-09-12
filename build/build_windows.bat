@echo off

cd "C:\Users\alexe\workspace\sptk5"
git reset --hard
git pull

cd "C:\Users\alexe\workspace\sptk5\build" 2>&1 > build.log

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" 2>&1 >> build.log
if errorlevel 1 (
    type build.log
    exit /b %errorlevel%
)

echo "Create build64 directory"
rmdir /S /Q build64 2>&1 >> build.log

mkdir build64 2>&1 >> build.log
cd build64
if errorlevel 1 (
    echo "Can't switch to build directory"
    exit /b %errorlevel%
)

echo "Configuring project"
cmake -G "Visual Studio 17 2022" -A x64 -DUSE_GTEST=OFF -DBUILD_EXAMPLES=OFF "C:\Users\alexe\workspace\sptk5\code"
if errorlevel 1 (
    echo "Can't configure build"
    exit /b %errorlevel%
)
cd ..

echo "Building project"
cmake --build build64 --config Release --target INSTALL
if errorlevel 1 (
    echo "Can't complete build and install"
    exit /b %errorlevel%
)

"C:\Program Files (x86)\Caphyon\Advanced Installer 18.2\bin\x86\advinst.exe" /build SPTK.aip 2>&1 >> build.log
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

scp -P 443 Downloads\SPTK-%VERSION%.exe alexeyp@www.sptk.net:/var/www/sites/sptk/download/%VERSION%/windows/SPTK-%VERSION%.exe
