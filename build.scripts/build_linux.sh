BUILD_ROOT=$(pwd)

cd $BUILD_ROOT/git/sptk5
git reset --hard && git pull > /dev/null

cd $BUILD_ROOT/git/xmq
git reset --hard && git pull > /dev/null

cd $BUILD_ROOT
SPTK_DIR=SPTK-5.6.1
XMQ_DIR=XMQ-0.9.5
rsync -av git/sptk5/code/ $SPTK_DIR > /dev/null
rsync -av git/xmq/ $XMQ_DIR > /dev/null

for dname in /home/alexeyp/Docker/Dockerfile.*
#for dname in /home/alexeyp/Docker/Dockerfile.fedora41
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh SPTK XMQ
done

rsync -av /build/output/$SPTK_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
rsync -av /build/output/$XMQ_DIR/* /var/www/html/sptk/download/$SPTK_DIR/