BUILD_ROOT=$(pwd)

TESTS=""
if [ "$1" = "--no-tests" ]; then
    TESTS=$1
fi

if [ -f XMQ_VERSION ]; then
    XMQ_VERSION=$(cat XMQ_VERSION)
    SPTK_VERSION=$(cat SPTK_VERSION)
else
    XMQ_VERSION=$(cat scripts/XMQ_VERSION)
    SPTK_VERSION=$(cat scripts/SPTK_VERSION)
fi

SPTK_DIR=SPTK-$SPTK_VERSION
XMQ_DIR=XMQ-$XMQ_VERSION

echo "$(date +%H:%M:%S) Remove build directories: $SPTK_DIR $XMQ_DIR"
sudo rm -rf $SPTK_DIR $XMQ_DIR > /tmp/op.log || (cat /tmp/op.log; exit 1)

echo "$(date +%H:%M:%S) Update SPTK from git"
cd $BUILD_ROOT/git/sptk5/
git pull  > /tmp/op.log || (cat /tmp/op.log; exit 1)
git checkout $SPTK_VERSION > /tmp/op.log || (cat /tmp/op.log; exit 1)

echo "$(date +%H:%M:%S) Update XMQ from git"
cd $BUILD_ROOT/git/xmq
git pull > /tmp/op.log || (cat /tmp/op.log; exit 1)
git checkout $XMQ_VERSION > /tmp/op.log || (cat /tmp/op.log; exit 1)

echo "$(date +%H:%M:%S) Create build directories: $SPTK_DIR $XMQ_DIR"
cd $BUILD_ROOT
rsync -av git/sptk5/code/ $SPTK_DIR > /tmp/op.log || (cat /tmp/op.log; exit 1)
rsync -av git/xmq/ $XMQ_DIR > /tmp/op.log || (cat /tmp/op.log; exit 1)

rm -f logs/*.log

#for dname in /home/alexeyp/Docker/Dockerfile.*
#for dname in /home/alexeyp/Docker/Dockerfile.ubuntu-25.10
for dname in /home/alexeyp/Docker/Dockerfile.debian-forky
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    echo "$(date +%H:%M:%S) Building $name"
    # Docker's default seccomp profile denies io_uring_setup/enter/register, so a containerised
    # build cannot exercise io_uring at all. Applied only when the profile is actually present:
    # this script is checked out along with the version being built, and older versions carry no
    # profile - naming a file that is not there makes the daemon refuse to start the container.
    SECCOMP=""
    if [ -f /build/scripts/seccomp-io-uring.json ]; then
        SECCOMP="--security-opt seccomp=/build/scripts/seccomp-io-uring.json"
    fi

    # No -t: allocating a TTY makes the run fail outright whenever stdin is not a terminal
    # ("cannot attach stdin to a TTY-enabled container"), which is every detached or scripted
    # invocation. A batch build has no use for one.
    # 2>&1 with it: without it only stdout went to the log and every tool's stderr - npm's
    # deprecation warnings above all - came out on the console, on top of the one line this loop
    # means to print per image.
    # test_http_host is 127.0.0.1 on this machine, which inside a container is the container's own
    # loopback with nothing listening on it - so the socket and TLS tests failed to resolve the
    # name and were reported as broken sockets. host-gateway points it at the host, where the web
    # server they mean to reach actually is.
    # Named, because without --name Docker invents one - loving_neumann, priceless_cori - and a
    # container left running after a build that moved on is then a stranger nobody recognises.
    # Two of those were found today, each holding a hung test suite and an hour of memory.
    docker rm -f xmq-build-$name > /dev/null 2>&1
    docker run --rm --name xmq-build-$name -v /build:/build $SECCOMP \
               --add-host test_http_host:host-gateway \
               -i builder-$name /build/scripts/build-package-cmake.sh $TESTS SPTK XMQ > logs/build-$name.log 2>&1
    echo BUILD RC=$?
done

echo "$(date +%H:%M:%S) Building complete"

# What failed, said once, at the end. Every suite already wrote a marker when it failed and nothing
# ever read one - so a night in which XMQ's tests ran on no image at all passed for a clean run.
# They were being invoked by name from /usr/local/bin, and XMQ had stopped installing its test
# binary; the builds themselves were fine, which is exactly why nobody looked.
failures=$(ls logs/*_failed.*.log 2>/dev/null)
if [ -n "$failures" ]; then
    echo
    echo "TEST SUITES THAT FAILED:"
    for marker in $failures; do
        echo "  $(basename $marker .log | sed -re 's/_failed\./ on /') - see $(cat $marker)"
    done
    echo
else
    echo "All test suites passed."
fi

rsync -qav /build/output/$SPTK_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
rsync -qav /build/output/$XMQ_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
