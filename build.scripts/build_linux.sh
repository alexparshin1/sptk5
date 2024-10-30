BUILD_ROOT=$(pwd)

TESTS=""
if [ "$1" = "--no-tests" ]; then
    TESTS=$1
fi

SPTK_VERSION=5.6.3
XMQ_VERSION=0.9.8
SPTK_DIR=SPTK-$SPTK_VERSION
XMQ_DIR=XMQ-$XMQ_VERSION

sudo rm -rf $SPTK_DIR $XMQ_DIR

cd $BUILD_ROOT/git/sptk5/
git pull > /dev/null
git checkout $SPTK_VERSION || exit 1

cd $BUILD_ROOT/git/xmq
git pull > /dev/null
git checkout $XMQ_VERSION || exit 1

cd $BUILD_ROOT
rsync -av git/sptk5/code/ $SPTK_DIR > /dev/null
rsync -av git/xmq/ $XMQ_DIR > /dev/null

rm -f logs/*.log

#for dname in /home/alexeyp/Docker/Dockerfile.*
for dname in /home/alexeyp/Docker/Dockerfile.ubuntu-numbat
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh $TESTS SPTK XMQ
done

rsync -qav /build/output/$SPTK_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
rsync -qav /build/output/$XMQ_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
