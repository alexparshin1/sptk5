#!/bin/sh

#sysctl vsyscall=emulate

export PATH=/opt/cov-analysis-linux64-2017.07/bin:$PATH

./distclean.sh
CC=clang CXX=clang++ cmake .
make clean

cov-build --dir cov-int make -j 8
rc=$?
if [ $rc != 0 ]; then
    tail cov-int/build-log.txt
    exit 1
fi

exit 1

tar czvf upload/sptk.tgz cov-int

curl --form token=uRFUaePWkebk8cxkxbeUCg \
  --form email=alexeyp@gmail.com \
  --form file=@upload/sptk.tgz \
  --form version="5.2.14" \
  --form description="SPTK" \
  -o coverity-upload.log --progress-bar \
  https://scan.coverity.com/builds?project=SPTK
