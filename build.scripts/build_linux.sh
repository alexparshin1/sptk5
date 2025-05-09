BUILD_ROOT=$(pwd)

TESTS=""
if [ "$1" = "--no-tests" ]; then
    TESTS=$1
fi

SPTK_VERSION=5.6.4
XMQ_VERSION=0.9.11

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

for dname in /home/alexeyp/Docker/Dockerfile.*
#for dname in /home/alexeyp/Docker/Dockerfile.ubuntu-oriole
#for dname in /home/alexeyp/Docker/Dockerfile.oraclelinux-9.5
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    echo "$(date +%H:%M:%S) Building $name"
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh $TESTS SPTK XMQ > logs/build-$name.log
done

echo "$(date +%H:%M:%S) Building complete"

rsync -qav /build/output/$SPTK_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
rsync -qav /build/output/$XMQ_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
