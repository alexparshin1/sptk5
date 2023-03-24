#!/bin/bash

OS_NAME=$(grep -E "^ID=" /etc/os-release | sed -re 's/^ID=//; s/"//g')
OS_VERSION=$(grep -E "^VERSION_ID=" /etc/os-release | sed -re 's/^VERSION_ID=//; s/"//g')
PLATFORM=$(grep -E '^PLATFORM_ID=' /etc/os-release | sed -re 's/^.*:(\w+).*$/\1/')  #'

echo $OS_NAME $OS_VERSION
echo

VERSION=$(head -1 /build/scripts/VERSION)
RELEASE="1"
PACKAGE_NAME="SPTK-$VERSION"

case $OS_NAME in
    ubuntu)
        OS_TYPE="ubuntu-$OS_VERSION"
        ;;

    ol)
        OS_TYPE="$PLATFORM"
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

echo OS_NAME:  $OS_NAME
echo PLATFORM: $PLATFORM

cd /build/$PACKAGE_NAME

CWD=`pwd`
./distclean.sh

src_name="/build/output/${VERSION}/sptk_${VERSION}"
[ ! -f ${src_name}.tgz ] && tar zcf ${src_name}.tgz --exclude-from=exclude_from_tarball.lst * > make_src_archives.log
[ ! -f ${src_name}.zip ] && zip -r ${src_name}.zip * --exclude '@exclude_from_tarball.lst' > make_src_archives.log

#cmake . -DCMAKE_INSTALL_PREFIX=/usr -DUSE_GTEST=OFF -DBUILD_EXAMPLES=OFF -DUSE_NEW_ABI=OFF && make -j4 package || exit 1
cmake . -DCMAKE_INSTALL_PREFIX=/usr/local -DUSE_GTEST=ON -DINSTALL_GTEST=ON -DBUILD_EXAMPLES=OFF -DUSE_NEW_ABI=OFF && make -j6 package install || exit 1
mkdir -p /build/output/$VERSION/ && chmod 777 /build/output/$VERSION/ || exit 1

for fname in *.rpm *.deb
do
    name=$(echo $fname | sed -re 's/SPTK.*Linux-/sptk-/' | sed -re "s/\.([a-z]+)$/-$VERSION.$OS_TYPE.\1/") #"
    mv $fname /build/output/$VERSION/$name
done

./distclean.sh

export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/opt/oracle/instantclient_18_3:${LD_LIBRARY_PATH}
echo "10.1.1.242  oracledb dbhost_oracle dbhost_mssql dbhost_pg dbhost_mysql smtp_host" >> /etc/hosts

cat /etc/hosts
cd test && /usr/local/bin/sptk_unit_tests 2>&1 > /build/farm/logs/unit_tests.$OS_TYPE.log  # --gtest_filter=SPTK_Oracle*
RC=$?

exit $RC
