#!/bin/bash

export PATH=/opt/sonar-scanner/bin:$PATH

which debuild > /dev/null || sudo apt install build-essential devscripts debhelper

VERSION="5.2.19"

BUILD_HOME=$(pwd)
cd ../code || exit 1

./distclean.sh

cmake . -DCMAKE_INSTALL_PREFIX=/usr

cd ..
tar zcf sptk_${VERSION}.orig.tar.gz code || exit 1
cd code || exit 1

pwd
debuild -us -uc -b

cd ${BUILD_HOME}

cp ../sptk_${VERSION}_amd64.deb /var/www/sites/sptk/download/
