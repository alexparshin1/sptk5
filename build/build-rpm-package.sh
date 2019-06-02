#!/bin/sh

VERSION=$(head -1 VERSION)
RELEASE="1"
PACKAGE_NAME="SPTK-$VERSION"

CORES=$(grep processor /proc/cpuinfo | wc -l)
CORES=$(($CORES-1))

cat sptk5.spec.src | sed -r "s/\{\{CORES\}\}/$CORES/" | sed -r "s/\{\{VERSION\}\}/$VERSION/" | sed -r "s/\{\{RELEASE\}\}/$RELEASE/" > sptk5.spec

echo Copy sources to temp directory
mkdir -p /tmp/$PACKAGE_NAME
rsync -a . /tmp/$PACKAGE_NAME/

echo Make source distribution archive
CWD=`pwd`
cd /tmp/$PACKAGE_NAME/
rm -rf .git sptk5.tar.gz
./distclean.sh
cd ..
tar zcf $CWD/sptk5.tar.gz $PACKAGE_NAME
cd $CWD

echo Prepare RPM build environment
mkdir -p ~/rpmbuild/{SOURCES,RPMS/x86_64}/
mv sptk5.tar.gz ~/rpmbuild/SOURCES/
rm ~/rpmbuild/RPMS/x86_64/sptk5*.rpm

echo Build RPM
rpmbuild -bb sptk5.spec

cp ~/rpmbuild/RPMS/x86_64/sptk5*.rpm .

