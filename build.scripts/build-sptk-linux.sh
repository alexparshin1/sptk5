BUILD_ROOT=$(pwd)

SPTK_DIR=SPTK-5.6.1

cd $BUILD_ROOT/git/sptk5/
git pull > /dev/null

cd $BUILD_ROOT
rsync -av git/sptk5/code/ $SPTK_DIR > /dev/null

#for dname in /home/alexeyp/Docker/Dockerfile.*
for dname in /home/alexeyp/Docker/Dockerfile.debian-bookworm
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh SPTK
done

rsync -av /build/output/$SPTK_DIR/* /var/www/html/sptk/download/$SPTK_DIR/
