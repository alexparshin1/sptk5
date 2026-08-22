#!/bin/sh

# GNU make is required to run the CMake-generated Makefiles (they use GNU-only syntax).
# Linux's default `make` already is GNU make; BSD's default `make` is bmake, so prefer
# `gmake` there when it's installed.
if command -v gmake >/dev/null 2>&1; then
    MAKE=gmake
else
    MAKE=make
fi

[ -f Makefile ] && $MAKE clean

[ -d _CPack_Packages ] && rm -rf _CPack_Packages

# POSIX/BSD find requires an explicit starting path (GNU find defaults it to '.'), and
# `-exec ... +` avoids collecting matches through a subshell/word-splitting loop.
find . -name CMakeFiles -type d -exec rm -rf {} +

[ -f CMakeCache.txt ] && rm -rf CMakeCache.txt

rm -rf Debug DebugCoverage Release

find . \( \
    -name Makefile -o \
    -name install_manifest.txt -o \
    -name cmake_install.cmake -o \
    -name cmake_uninstall.cmake -o \
    -name compile_commands.json -o \
    -name '*.deb' -o \
    -name 'install_manifest*.txt' \
  \) -exec rm -rf {} +

if [ -d lib ]; then
  rm -rf lib/*
fi
