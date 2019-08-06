#!/bin/bash

cat /etc/redhat-release
echo

VERSION=$(head -1 /build/scripts/VERSION)
RELEASE="1"
PACKAGE_NAME="SPTK-$VERSION"

CORES=$(grep processor /proc/cpuinfo | wc -l)
CORES=$(($CORES-1))

cd /build/$PACKAGE_NAME

cat /build/scripts/sptk.spec.src | sed -r "s/\{\{CORES\}\}/$CORES/" | sed -r "s/\{\{VERSION\}\}/$VERSION/" | sed -r "s/\{\{RELEASE\}\}/$RELEASE/" > sptk.spec

echo Make source distribution archive
CWD=`pwd`
./distclean.sh
cd ..
tar zcf sptk-$VERSION.tar.gz $PACKAGE_NAME

echo Build RPM
cd $CWD
make -j4 package

#cp ~/rpmbuild/RPMS/x86_64/*.rpm /build/output/

