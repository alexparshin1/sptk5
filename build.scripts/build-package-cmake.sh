#!/bin/bash

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export PATH=/usr/local/bin:$PATH
export CMAKE_COLOR_DIAGNOSTICS=OFF

rsync -avz /build/etc/xmq /etc/

RUN_TESTS="true"
if [ "$1" = "--no-tests" ]; then
    RUN_TESTS="false"
    shift
fi

# Build scroipt for building either SPTK or XMQ packages in Docker environment

for PACKAGE in $@; do

echo ═════════════════════════════ $PACKAGE ═══════════════════════════════

if [ ! "$PACKAGE" = "SPTK" ] && [ ! "$PACKAGE" = "XMQ" ]; then
    echo "Please provide package name, SPTK or XMQ"
    exit 1
fi

OS_NAME=$(grep -E "^ID=" /etc/os-release | sed -re 's/^ID=//; s/"//g')
OS_VERSION=$(grep -E "^VERSION_ID=" /etc/os-release | sed -re 's/^VERSION_ID=//; s/"//g')
OS_CODENAME=$(grep -E '^VERSION_CODENAME=' /etc/os-release | sed -re 's/^.*=(\w+)?.*$/\1/')  #'
PLATFORM=$(grep -E '^PLATFORM_ID=' /etc/os-release | sed -re 's/^.*:(\w+).*$/\1/')  #'

if [ "$OS_VERSION" = "" ]; then
    OS_VERSION=$OS_CODENAME
fi

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
    debian)
        OS_TYPE="debian-$OS_VERSION"
        ;;

    ubuntu)
        OS_TYPE="ubuntu-$OS_VERSION"
        ;;

    ol)
        OS_TYPE="$PLATFORM"
        # The major version alone, because the packages are binary compatible across an Enterprise
        # Linux major release: one directory serves RHEL, Alma, Rocky and Oracle of that major.
        #
        # This was hardcoded to "oraclelinux-9.5" and stayed there when the image moved to Oracle
        # Linux 10, so EL10 packages were published under a name promising EL9. They ask for
        # glibc 2.38 and EL9 has 2.34, so anyone who followed that name downloaded something that
        # could not install. Derived from the running system now, like every other distribution
        # here.
        DOWNLOAD_DIRNAME="$OS_FULLNAME-${OS_VERSION%%.*}"
        ;;

    fedora)
        OS_TYPE="fc$OS_VERSION"
        ;;
esac

echo OS_NAME:    $OS_NAME
echo PLATFORM:   $PLATFORM
echo PACKAGE:    $PACKAGE_NAME
echo ──────────────────────────────────────────────────────────────────
cd /build/$PACKAGE_NAME || exit

CWD=`pwd`
./distclean.sh

if [ $PACKAGE = "SPTK" ]; then
    TAR_DIR="/build/output/${PACKAGE}-${VERSION}/tar"
    mkdir -p "${TAR_DIR}"
    src_name="${TAR_DIR}/${PACKAGE_NAME}"
    echo "Base tar name: ${src_name}" > make_src_archives.log
    [ ! -f ${src_name}.txz ] && tar Jcf ${src_name}.txz --exclude-from=exclude_from_tarball.lst * >> make_src_archives.log
    [ ! -f ${src_name}.zip ] && zip -r ${src_name}.zip * --exclude '@exclude_from_tarball.lst' >> make_src_archives.log
fi

if [ $PACKAGE = "SPTK" ]; then
    BUILD_OPTIONS="-DUSE_GTEST=ON -DBUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=/usr/local"
else
    BUILD_OPTIONS="-DCMAKE_INSTALL_PREFIX=/usr/local"
fi

sh ./distclean.sh
ulimit -n 16384
cmake . $BUILD_OPTIONS -DCMAKE_BUILD_TYPE=Release || exit 1

# Both steps into the log, and stderr with them. Two things were wrong here: only the packaging
# step was redirected, so everything "make install" built - the React interface among it - reported
# to the console; and "2>&1 > file" is the wrong way round, duplicating stderr to the console
# *before* sending stdout to the file, which is why every warning any tool produced ended up on
# screen. A packaging run should show its own progress and nothing else.
# Named from $PACKAGE here, where it is known. It used to be assigned further down, inside the
# loop that renames the produced package files - so the build log above it was written to
# "_build.<os>.log" with the name missing, and SPTK's and XMQ's overwrote one another.
lcPACKAGE=$(echo "$PACKAGE" | tr '[:upper:]' '[:lower:]')

{ make -j8 install && make -j6 package ; } > /build/logs/${lcPACKAGE}_build.$OS_TYPE.log 2>&1 || exit 1

echo ──────────────────────────────────────────────────────────────────
BUILD_OUTPUT_DIR=/build/output/$PACKAGE-$VERSION
sh ./install_local_packages.sh
mkdir -p $BUILD_OUTPUT_DIR && chmod 777 $BUILD_OUTPUT_DIR || exit 1
echo ──────────────────────────────────────────────────────────────────

OUTPUT_DIR=$BUILD_OUTPUT_DIR/$DOWNLOAD_DIRNAME
mkdir -p $OUTPUT_DIR || exit 1
for fname in $(ls *.rpm *.deb)
do
    if [ $PACKAGE = "SPTK" ]; then
        name=$(echo $fname | sed -re 's/^SPTK/sptk/;s/-Linux//')
        lcPACKAGE="sptk"
    else
        name=$(echo $fname | sed -re 's/^XMQ/xmq/;s/-Linux//')
        lcPACKAGE="xmq"
    fi
    mv $fname $OUTPUT_DIR/$name
    # The checksum of what people actually download, beside it, in the format "shasum -a 256 -c"
    # reads. Written here and not where the package was built, because the file is renamed on the
    # way - XMQ becomes xmq and "-Linux" is dropped - so a checksum made any earlier would name a
    # file that never reaches the download area.
    ( cd $OUTPUT_DIR && sha256sum $name > $name.sha256 ) || exit 1
done

echo ──────────────────────────────────────────────────────────────────

# Before distclean, not after. The suite used to be run by bare name from /usr/local/bin, which
# worked only because "make install" put it there - and XMQ stopped installing its test binary when
# the packaging work took it out of the .deb and .rpm, where it had been dragging googletest into a
# production package. Nothing said so: the suite simply never ran again, on any image, and the run
# reported "xmq_unit_tests: command not found" into a log nobody reads when the build itself is fine.
if [ $RUN_TESTS = "true" ]; then

    echo "┌──────────────────────────────────────────────────────────────────────────────┐"
    echo "│   Unit tests suite is starting.                                              │"
    echo "│                                                                              │"
    echo "│   Please note that log [ERROR] messages are expected in many unit tests.     │"
    echo "│   If a unit tests fails then GTEST indicates it with [FAIL] message.         │"
    echo "└──────────────────────────────────────────────────────────────────────────────┘"
    echo

    export PATH=/usr/local/bin:$PATH
    export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/opt/oracle/instantclient_18_3:${LD_LIBRARY_PATH}
    grep "10.1.1.242" /etc/hosts
    if [ $? == 1 ]; then
        echo "10.1.1.242  theater oracledb dbhost_oracle dbhost_mssql dbhost_pg dbhost_mysql smtp_host redis_server mosquitto_server" >> /etc/hosts
    fi

    cp /usr/share/zoneinfo/Australia/Melbourne /etc/localtime

    ulimit -n 32768
    # "> file 2>&1", not "2>&1 > file": the second form sends stderr to wherever stdout points at
    # the time, which is the console, and only then redirects stdout. Every test's error output was
    # going to the screen while the log recorded only the quiet half.
    # From the build tree by preference, falling back to whatever is on PATH. SPTK's binary carries
    # its version in the name - sptk_unit_tests-5.6.9 - and XMQ's does not, so the name is looked for
    # rather than assumed. An installed copy still answers for anyone who has one.
    cd $CWD/test || exit 1
    suite=$(ls -1 ${lcPACKAGE}_unit_tests ${lcPACKAGE}_unit_tests-* 2>/dev/null | head -1)
    if [ -n "$suite" ]; then
        suite="./$suite"
    else
        suite="${lcPACKAGE}_unit_tests"
    fi
    echo "Test suite: $suite"
    # Shuffled, with a seed taken from the clock and printed at the top of the log.
    #
    # A suite that only passes in one order proves less than it looks: on 2026-08-31 shuffling this
    # one turned up six tests that read the broker's own counters as though nothing else used them,
    # one of which cleared the shared statistics and sent a counter below zero for everything that
    # ran after it. All of them passed every run of the farm until then. Ubuntu 24.10 was dropped
    # from the farm years earlier over two or three failures nobody could explain, which is what
    # this kind of fault looks like from the outside.
    #
    # A different order every run, rather than a fixed seed, so the farm keeps looking; the seed in
    # the log is what makes a failure reproducible afterwards - pass it back with
    # --gtest_random_seed=N.
    $suite --gtest_filter=-*Scenario* --gtest_shuffle > /build/logs/${lcPACKAGE}_unit_tests.$OS_TYPE.log 2>&1
    RC=$?

    # The image is in the name. It used to be ${lcPACKAGE}_failed.log for every image in the run,
    # so nine images left one marker between them and each overwrote the last - a failure on the
    # first eight was erased by a pass on the ninth.
    if [ $RC != 0 ]; then
        echo "/build/logs/${lcPACKAGE}_unit_tests.$OS_TYPE.log" > /build/logs/${lcPACKAGE}_failed.$OS_TYPE.log
    else
        rm -f /build/logs/${lcPACKAGE}_failed.$OS_TYPE.log
    fi
fi

cd $CWD
sh ./distclean.sh
sh ./distclean.sh
chown -R alexeyp SPTK* XMQ*

done

exit $RC
