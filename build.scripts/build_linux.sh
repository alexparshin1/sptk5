BUILD_ROOT=$(pwd)

SPTK_DIR=SPTK-5.6.2
XMQ_DIR=XMQ-0.9.5

sudo rm -rf $SPTK_DIR $XMQ_DIR

cd $BUILD_ROOT/git/sptk5/
git pull > /dev/null

cd $BUILD_ROOT/git/xmq
git pull > /dev/null

cd $BUILD_ROOT
rsync -av git/sptk5/code/ $SPTK_DIR > /dev/null
rsync -av git/xmq/ $XMQ_DIR > /dev/null

#for dname in /home/alexeyp/Docker/Dockerfile.*
for dname in /home/alexeyp/Docker/Dockerfile.fedora37
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh SPTK XMQ
done

rsync -qav /build/output/$SPTK_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
rsync -qav /build/output/$XMQ_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
