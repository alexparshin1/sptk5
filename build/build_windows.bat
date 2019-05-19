cd "C:\Users\alexe\workspace\sptk5\build"

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

devenv ..\code\w64\SPTK.sln /build RelWithDebInfo /project INSTALL
