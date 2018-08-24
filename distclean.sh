#!/bin/bash

[ -f Makefile ] && make clean

rm CMakeCache.txt
for file in Makefile CMakeFiles install_manifest.txt cmake_install.cmake cmake_uninstall.cmake compile_commands.json
do
  find -name $file -exec rm -rf {} \;
done

rm lib/*
