BUILD_ROOT=$(pwd)

cd git/sptk5
git reset --hard && git pull > /dev/null

cd git/smq
git reset --hard && git pull > /dev/null

cd $BUILD_ROOT
rsync -av git/sptk5/code/ SPTK-5.6.0 > /dev/null
rsync -av git/smq/ SMQ-0.9.0 > /dev/null

for dname in /home/alexeyp/Docker/Dockerfile.*
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh SPTK1
done

rsync -av /build/output/* /var/www/html/sptk/download/