#!/bin/bash

sudo apt install build-essential devscripts debhelper

VERSION="5.2.11"

cmake . -DCMAKE_INSTALL_PREFIX=/usr

#./distclean.sh
cd ..
tar zcf sptk_$VERSION.orig.tar.gz sptk5 || exit 1
cd sptk5 || exit 1

debuild -us -uc
