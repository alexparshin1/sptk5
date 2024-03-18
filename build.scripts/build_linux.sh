BUILD_ROOT=$(pwd)

#cd $BUILD_ROOT/git/sptk5
#git reset --hard && git pull > /dev/null

#cd $BUILD_ROOT/git/xmq
#git reset --hard && git pull > /dev/null

cd $BUILD_ROOT
#rsync -av git/sptk5/code/ SPTK-5.6.0 > /dev/null
#rsync -av git/xmq/ XMQ-0.9.0 > /dev/null

#for dname in /home/alexeyp/Docker/Dockerfile.*
for dname in /home/alexeyp/Docker/Dockerfile.fedora35
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh SPTK XMQ
done

rsync -av /build/output/* /var/www/html/sptk/download/