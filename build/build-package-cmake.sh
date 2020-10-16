#!/bin/bash

OS_NAME=$(grep -E "^ID=" /etc/os-release | sed -re 's/^ID=//; s/"//g')
OS_VERSION=$(grep -E "^VERSION_ID=" /etc/os-release | sed -re 's/^VERSION_ID=//; s/"//g')

echo $OS_NAME $OS_VERSION
echo

VERSION=$(head -1 /build/scripts/VERSION)
RELEASE="1"
PACKAGE_NAME="SPTK-$VERSION"

case $OS_NAME in
    ubuntu)
        OS_TYPE="ubuntu-$OS_VERSION"
        ;;

    centos)
        OS_TYPE="el$OS_VERSION"
        ;;

    fedora)
        OS_TYPE="fc$OS_VERSION"
        ;;

    *)
        OS_TYPE="$OS_NAME-$OS_VERSION"
        ;;
esac

cd /build/$PACKAGE_NAME

CWD=`pwd`
./distclean.sh

src_name="/build/output/${VERSION}/sptk_${VERSION}"
[ ! -f ${src_name}.tgz ] && tar zcf ${src_name}.tgz --exclude-from=exclude_from_tarball.lst *
[ ! -f ${src_name}.zip ] && zip -r ${src_name}.zip * --exclude '@exclude_from_tarball.lst'

cmake . -DCMAKE_INSTALL_PREFIX=/usr -DUSE_GTEST=OFF -DBUILD_EXAMPLES=OFF -DUSE_NEW_ABI=OFF && make -j4 package
mkdir -p /build/output/$VERSION/
chmod 777 /build/output/$VERSION/

for fname in *.rpm *.deb
do
    name=$(echo $fname | sed -re 's/SPTK.*Linux-/sptk-/' | sed -re "s/\.([a-z]+)$/-$VERSION.$OS_TYPE.\1/") #"
    mv $fname /build/output/$VERSION/$name
done

./distclean.sh
