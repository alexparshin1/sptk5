REM Build SPTK installer in Windows
REM @echo off

if "%~1" == "" (
    PACKAGE="sptk5"
) else (
    PACKAGE="%~1"
)

cd "C:\workspace\%PACKAGE%"
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

REM The project at the root of the tree, which is the one that is maintained - 278 files and 24
REM prerequisites, against the 236 and none of the copy that used to be built here. Building the
REM other one quietly published an installer 14 MB smaller than the release before it. There is
REM one project now; the copy under build.scripts is gone.
"%ADVINST%" /build "Simply Powerful Toolkit.aip" >> build.log 2>&1
if errorlevel 1 (
    echo "Can't build installer"
    exit /b %errorlevel%
)

mkdir Downloads >> build.log 2>&1

set /p VERSION=<VERSION

REM Advanced Installer writes its output beside the .aip, so the directory is named after the
REM project and sits here at the root. The build produces SPTK.msi - the project is a plain MSI,
REM not the bootstrapper XMQ's is - so nothing called SPTK.exe was ever going to be found. And
REM "move", not "mv": this runs in cmd, where mv does not exist. Git for Windows ships one, in a
REM directory that is not on PATH.
move /Y "Simply Powerful Toolkit-SetupFiles\SPTK-%VERSION%.msi" Downloads\SPTK-%VERSION%.msi >> build.log 2>&1
if errorlevel 1 (
    echo "Can't move installer to Downloads directory"
    exit /b %errorlevel%
)

rmdir /S /Q "Simply Powerful Toolkit-SetupFiles" >> build.log 2>&1

echo Computing the checksum
REM Written in the format "shasum -a 256 -c" reads: the hash, two spaces, the name. Get-FileHash
REM rather than certutil, whose output has changed shape between Windows versions. No trailing
REM newline, so that no CR reaches a file that will be read on Linux - a CR there becomes part of
REM the file name and the check then fails looking for a file nobody has.
powershell -NoProfile -Command "$name = 'SPTK-%VERSION%.msi'; $hash = (Get-FileHash -Algorithm SHA256 (Join-Path 'Downloads' $name)).Hash.ToLower(); Set-Content -Path (Join-Path 'Downloads' ($name + '.sha256')) -Value ($hash + '  ' + $name) -NoNewline -Encoding ascii"
if errorlevel 1 (
    echo "Can't compute the checksum"
    exit /b %errorlevel%
)

REM The download area is laid out as SPTK-<version>, not <version>: this used to upload into
REM download/5.6.9/windows, a directory that does not exist, so the copy failed and no Windows
REM installer has been published since 5.6.7. The directory is created first, because a new
REM release has none.
REM The web host over the local network, because that is where this machine is. It used to be
REM www.sptk.net on 443, which answers from outside but closes the connection from here. Both are
REM overridable: set REMOTE_HOST and REMOTE_PORT before running to publish from somewhere else.
if not defined REMOTE_HOST set REMOTE_HOST=alexeyp@10.1.1.242
if not defined REMOTE_PORT set REMOTE_PORT=22
set REMOTE_DIR=/var/www/html/sptk/download/SPTK-%VERSION%/windows

ssh -p %REMOTE_PORT% %REMOTE_HOST% "mkdir -p %REMOTE_DIR%"
if errorlevel 1 (
    echo "Can't create the remote directory"
    exit /b %errorlevel%
)

scp -P %REMOTE_PORT% Downloads\SPTK-%VERSION%.msi Downloads\SPTK-%VERSION%.msi.sha256 %REMOTE_HOST%:%REMOTE_DIR%/
if errorlevel 1 (
    echo "Can't upload the installer"
    exit /b %errorlevel%
)
