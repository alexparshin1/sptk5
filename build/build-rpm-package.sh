#!/bin/bash

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

echo Prepare RPM build environment
mkdir -p ~/rpmbuild/{SOURCES,RPMS/x86_64}/
mv sptk-$VERSION.tar.gz ~/rpmbuild/SOURCES/
rm ~/rpmbuild/RPMS/x86_64/sptk5*.rpm

ls -l ~/rpmbuild/SOURCES/

echo Build RPM
cd $CWD
rpmbuild -bb sptk.spec

cp ~/rpmbuild/RPMS/x86_64/sptk*.rpm .

