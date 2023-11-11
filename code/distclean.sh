###!/bin/bash

[ -f Makefile ] && make clean

[ -d _CPack_Packages ] && rm -rf _CPack_Packages

for dname in $(find -name CMakeFiles -type d)
do
    [ -d $dname ] && rm -rf $dname
done

[ -f CMakeCache.txt ] && rm CMakeCache.txt Debug/CMakeCache.txt Release/CMakeCache.txt

for file in Makefile install_manifest.txt cmake_install.cmake cmake_uninstall.cmake compile_commands.json *.deb install_manifest*.txt
do
  find -name $file -exec rm -rf {} \;
done

if [ -d lib ]; then
  rm -rf lib/*
fi

