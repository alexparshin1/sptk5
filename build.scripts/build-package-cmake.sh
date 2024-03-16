#!/bin/bash

PACKAGE=$1
if [ ! "$PACKAGE" = "SPTK" ] && [ ! "$PACKAGE" = "SMQ" ]; then
    echo "Please provide package name, SPTK or SMQ"
    exit 1
fi

OS_NAME=$(grep -E "^ID=" /etc/os-release | sed -re 's/^ID=//; s/"//g')
OS_VERSION=$(grep -E "^VERSION_ID=" /etc/os-release | sed -re 's/^VERSION_ID=//; s/"//g')
OS_CODENAME=$(grep -E '^VERSION_CODENAME=' /etc/os-release | sed -re 's/^.*=(\w+)?.*$/\1/')  #'
PLATFORM=$(grep -E '^PLATFORM_ID=' /etc/os-release | sed -re 's/^.*:(\w+).*$/\1/')  #'

OS_FULLNAME=$OS_NAME
if [ "$OS_NAME" = "ol" ]; then
    OS_FULLNAME="oraclelinux"
fi

if [ "$OS_CODENAME" = "" ]; then
    OS_CODENAME=$OS_VERSION
fi

VERSION=$(head -1 /build/scripts/VERSION)
RELEASE="1"
PACKAGE_NAME="$PACKAGE-$VERSION"

DOWNLOAD_DIRNAME=$OS_NAME-$OS_CODENAME
OS_TYPE="$OS_NAME-$OS_VERSION"
case $OS_NAME in
    ubuntu)
        OS_TYPE="ubuntu-$OS_VERSION"
        ;;

    ol)
        OS_TYPE="$PLATFORM"
        DOWNLOAD_DIRNAME="oraclelinux9"
        ;;

    fedora)
        OS_TYPE="fc$OS_VERSION"
        ;;
esac

echo OS_NAME:  $OS_NAME
echo PLATFORM: $PLATFORM

cd /build/$PACKAGE_NAME

CWD=`pwd`
./distclean.sh

TAR_DIR="/build/output/${VERSION}/tar"
mkdir -p "$TAR_DIR"
src_name="$TAR_DIR/$PACKAGE_${VERSION}"
[ ! -f ${src_name}.tgz ] && tar zcf ${src_name}.tgz --exclude-from=exclude_from_tarball.lst * > make_src_archives.log
[ ! -f ${src_name}.zip ] && zip -r ${src_name}.zip * --exclude '@exclude_from_tarball.lst' > make_src_archives.log

#cmake . -DCMAKE_INSTALL_PREFIX=/usr -DUSE_GTEST=OFF -DBUILD_EXAMPLES=OFF -DUSE_NEW_ABI=OFF && make -j4 package || exit 1
cmake . -DCMAKE_INSTALL_PREFIX=/usr/local -DUSE_GTEST=ON -DINSTALL_GTEST=ON -DBUILD_EXAMPLES=OFF -DUSE_NEW_ABI=ON && make -j6 package install || exit 1
mkdir -p /build/output/$VERSION/ && chmod 777 /build/output/$VERSION/ || exit 1

OUTPUT_DIR=/build/output/$VERSION/$DOWNLOAD_DIRNAME
mkdir -p $OUTPUT_DIR || exit 1
for fname in *.rpm *.deb
do
    if [ $PACKAGE = "SPTK" ]; then
        name=$(echo $fname | sed -re 's/SPTK.*Linux-/sptk-/' | sed -re "s/\.([a-z]+)$/-$VERSION.$OS_TYPE.\1/") #"
        lcPACKAGE="sptk"
    else
        name=$(echo $fname | sed -re 's/SMQ.*Linux-/smq-/' | sed -re "s/\.([a-z]+)$/-$VERSION.$OS_TYPE.\1/") #"
        lcPACKAGE="smq"
    fi
    mv $fname $OUTPUT_DIR/$name
done

export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/opt/oracle/instantclient_18_3:${LD_LIBRARY_PATH}
echo "10.1.1.242  theater oracledb dbhost_oracle dbhost_mssql dbhost_pg dbhost_mysql smtp_host" >> /etc/hosts

cat /etc/hosts
pwd
cd $CWD/test && ./sptk_unit_tests 2>&1 > /build/farm/logs/unit_tests.$OS_TYPE.log  # --gtest_filter=SPTK_Oracle*
RC=$?

if [ $RC != 0 ]; then
    echo "/build/farm/logs/${lcPACKAGE}_unit_tests.$OS_TYPE.log" > /build/farm/logs/${lcPACKAGE}_failed.log
else
    rm /build/farm/logs/${lcPACKAGE}_failed.log
fi

cd $CWD
./distclean.sh

exit $RC
