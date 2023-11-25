BUILD_ROOT=$(pwd)

cd sptk5
git reset --hard && git pull > /dev/null

cd $BUILD_ROOT
rsync -av sptk5/code/ SPTK-5.6.0 > /dev/null

for dname in /home/alexeyp/Docker/Dockerfile.*
do
    name=$(echo $dname | sed -re 's/^.*Dockerfile.//')
    docker run --rm -v /build:/build -it builder-$name /build/scripts/build-package-cmake.sh
done