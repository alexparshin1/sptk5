#!/bin/bash

for PACKAGE in $@; do

if [ ! "$PACKAGE" = "SPTK" ] && [ ! "$PACKAGE" = "XMQ" ]; then
    echo "Please provide package name, SPTK or XMQ"
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

VERSION=$(head -1 /build/scripts/${PACKAGE}_VERSION)
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

echo
echo ──────────────────────────────────────────────────────────────────
echo OS_NAME:   $OS_NAME
echo PLATFORM:  $PLATFORM
echo PACKAGE:   $PACKAGE_NAME
echo ──────────────────────────────────────────────────────────────────
cd /build/$PACKAGE_NAME || exit

CWD=`pwd`
./distclean.sh

TAR_DIR="/build/output/${VERSION}/tar"
mkdir -p "$TAR_DIR"
src_name="$TAR_DIR/$PACKAGE_${VERSION}"
[ ! -f ${src_name}.tgz ] && tar zcf ${src_name}.tgz --exclude-from=exclude_from_tarball.lst * > make_src_archives.log
[ ! -f ${src_name}.zip ] && zip -r ${src_name}.zip * --exclude '@exclude_from_tarball.lst' > make_src_archives.log

if [ $PACKAGE = "SPTK" ]; then
    BUILD_OPTIONS="-DUSE_GTEST=ON -DINSTALL_GTEST=ON -DBUILD_EXAMPLES=OFF"
else
    BUILD_OPTIONS=""
fi

cmake . -DCMAKE_INSTALL_PREFIX=/usr/local $BUILD_OPTIONS -DUSE_NEW_ABI=ON && make -j6 package || exit 1
./install_local_packages.sh
mkdir -p /build/output/$VERSION/ && chmod 777 /build/output/$VERSION/ || exit 1

ls -l /usr/lib/libsp*
ls -l /usr/include/sptk5

OUTPUT_DIR=/build/output/$VERSION/$DOWNLOAD_DIRNAME
mkdir -p $OUTPUT_DIR || exit 1
for fname in *.rpm *.deb
do
    if [ $PACKAGE = "SPTK" ]; then
        name=$(echo $fname | sed -re 's/SPTK.*Linux-/sptk-/' | sed -re "s/\.([a-z]+)$/-$VERSION.$OS_TYPE.\1/") #"
        lcPACKAGE="sptk"
    else
        name=$(echo $fname | sed -re 's/XMQ.*Linux-/xmq-/' | sed -re "s/\.([a-z]+)$/-$VERSION.$OS_TYPE.\1/") #"
        lcPACKAGE="xmq"
    fi
    mv $fname $OUTPUT_DIR/$name
done

export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/opt/oracle/instantclient_18_3:${LD_LIBRARY_PATH}
echo "10.1.1.242  theater oracledb dbhost_oracle dbhost_mssql dbhost_pg dbhost_mysql smtp_host" >> /etc/hosts

cat /etc/hosts
pwd
cd $CWD/test && ./${lcPACKAGE}_unit_tests 2>&1 > /build/farm/logs/${lcPACKAGE}_unit_tests.$OS_TYPE.log
RC=$?

if [ $RC != 0 ]; then
    echo "/build/farm/logs/${lcPACKAGE}_unit_tests.$OS_TYPE.log" > /build/farm/logs/${lcPACKAGE}_failed.log
else
    rm /build/farm/logs/${lcPACKAGE}_failed.log
fi

cd $CWD
./distclean.sh

done

exit $RC
