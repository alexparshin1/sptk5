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
for dname in /home/alexeyp/Docker/Dockerfile.oraclelinux-9.5
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    echo "$(date +%H:%M:%S) Building $name"
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh $TESTS SPTK XMQ > logs/build-$name.log
done

echo "$(date +%H:%M:%S) Building complete"

rsync -qav /build/output/$SPTK_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
rsync -qav /build/output/$XMQ_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
