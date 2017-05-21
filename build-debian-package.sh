#!/bin/bash

which debuild > /dev/null || sudo apt install build-essential devscripts debhelper

VERSION="5.2.13"

cmake . -DCMAKE_INSTALL_PREFIX=/usr

cd ..
tar zcf sptk_$VERSION.orig.tar.gz sptk5 || exit 1
cd sptk5 || exit 1

debuild -us -uc -b
