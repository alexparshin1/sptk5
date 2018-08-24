#!/bin/sh

CWD=$(pwd)

cd googletest || exit 1
[ ! -f CMakeLists.txt.orig ] && cp CMakeLists.txt CMakeLists.txt.orig

cat CMakeLists.txt.orig | \
sed -re 's|^option\(BUILD_SHARED_LIBS.*$|set(BUILD_SHARED_LIBS ON)|' \
> CMakeLists.txt
