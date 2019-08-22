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
        OS_TYPE="el7"
        ;;

    *)
        OS_TYPE="$OS_NAME-$OS_VERSION"
        ;;
esac

cd /build/$PACKAGE_NAME

CWD=`pwd`
./distclean.sh

cmake . && make -j4 package
mkdir -p /build/output/$VERSION/

for fname in *.rpm
do
    name=$(echo $fname | sed -re 's/SPTK.*Linux-/sptk-/' | sed -re "s/\.([a-z]+)$/-$VERSION.$OS_TYPE.\1/") #"
    mv $fname /build/output/$VERSION/$name
done

./distclean.sh
