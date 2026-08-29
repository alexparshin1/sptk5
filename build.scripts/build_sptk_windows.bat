REM Build SPTK installer in Windows
REM @echo off

cd "C:\workspace\sptk5"
git reset --hard
git pull

call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"

if errorlevel 1 (
    type build.log
    exit /b %errorlevel%
)

echo Remove old build64 directory
rmdir /S /Q build
REM >> build.log 2>&1

echo Create build directory
mkdir build >> build.log 2>&1
cd build
if errorlevel 1 (
    echo "Can't switch to build directory"
    exit /b %errorlevel%
)

echo Configuring project
cmake -G "Visual Studio 18 2026" -A x64 -DUSE_GTEST=OFF -DBUILD_EXAMPLES=OFF "C:\workspace\sptk5\code"
if errorlevel 1 (
    echo "Can't configure build"
    exit /b %errorlevel%
)
cd ..

echo "Building project"
cmake --build build --config Release --target INSTALL --parallel 4
if errorlevel 1 (
    echo "Can't complete build and install"
    exit /b %errorlevel%
)

REM Found rather than named. The version was written in here as 23.3 while 23.9 is what is
REM installed, so this step ran a path that does not exist and the build stopped here every time.
REM Highest name wins, which is right until there is a 23.10 to sort below 23.9 - pin ADVINST in
REM the environment if that day comes.
set ADVINST=
for /f "delims=" %%d in ('dir /b /o-n "C:\Program Files (x86)\Caphyon\Advanced Installer *" 2^>nul') do (
    if not defined ADVINST if exist "C:\Program Files (x86)\Caphyon\%%d\bin\x86\advinst.exe" set "ADVINST=C:\Program Files (x86)\Caphyon\%%d\bin\x86\advinst.exe"
)
if not defined ADVINST (
    echo "Advanced Installer not found under C:\Program Files (x86)\Caphyon"
    exit /b 1
)
echo Building the installer with %ADVINST%

"%ADVINST%" /build build.scripts\SPTK.aip >> build.log 2>&1
if errorlevel 1 (
    echo "Can't build installer"
    exit /b %errorlevel%
)

mkdir Downloads >> build.log 2>&1

set /p VERSION=<VERSION

REM move, not mv: this runs in cmd, where mv does not exist. Git for Windows has one, in a
REM directory that is not on PATH, so the line failed and the installer never reached Downloads.
move /Y SPTK-SetupFiles\SPTK.exe Downloads\SPTK-%VERSION%.exe >> build.log 2>&1
if errorlevel 1 (
    echo "Can't move installer to Downloads directory"
    exit /b %errorlevel%
)

rmdir /S /Q SPTK-SetupFiles SPTK-cache >> build.log 2>&1

echo Computing the checksum
REM Written in the format "shasum -a 256 -c" reads: the hash, two spaces, the name. Get-FileHash
REM rather than certutil, whose output has changed shape between Windows versions. No trailing
REM newline, so that no CR reaches a file that will be read on Linux - a CR there becomes part of
REM the file name and the check then fails looking for a file nobody has.
powershell -NoProfile -Command "$name = 'SPTK-%VERSION%.exe'; $hash = (Get-FileHash -Algorithm SHA256 (Join-Path 'Downloads' $name)).Hash.ToLower(); Set-Content -Path (Join-Path 'Downloads' ($name + '.sha256')) -Value ($hash + '  ' + $name) -NoNewline -Encoding ascii"
if errorlevel 1 (
    echo "Can't compute the checksum"
    exit /b %errorlevel%
)

REM The download area is laid out as SPTK-<version>, not <version>: this used to upload into
REM download/5.6.9/windows, a directory that does not exist, so the copy failed and no Windows
REM installer has been published since 5.6.7. The directory is created first, because a new
REM release has none.
set REMOTE_HOST=alexeyp@www.sptk.net
set REMOTE_DIR=/var/www/html/sptk/download/SPTK-%VERSION%/windows

ssh -p 443 %REMOTE_HOST% "mkdir -p %REMOTE_DIR%"
if errorlevel 1 (
    echo "Can't create the remote directory"
    exit /b %errorlevel%
)

scp -P 443 Downloads\SPTK-%VERSION%.exe Downloads\SPTK-%VERSION%.exe.sha256 %REMOTE_HOST%:%REMOTE_DIR%/
if errorlevel 1 (
    echo "Can't upload the installer"
    exit /b %errorlevel%
)
