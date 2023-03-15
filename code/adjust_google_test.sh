#!/bin/sh

CWD=$(pwd)

cd googletest || exit 1
[ ! -f CMakeLists.txt.orig ] && cp CMakeLists.txt CMakeLists.txt.orig

cat CMakeLists.txt.orig | \
sed -re 's|^option\(BUILD_SHARED_LIBS.*$|set(BUILD_SHARED_LIBS ON)|' \
| sed -re 's|DESTINATION \$\{cmake_files_install_dir\}\)|RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)|i' \
> CMakeLists.txt
