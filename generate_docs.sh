#!/bin/bash

SPTKDIR=`pwd`

CXX_INCLUDE_DIR=$(gcc -v 2>&1 | grep 'Configured with:' | sed -r 's/^.*--with-gxx-include-dir=(\S+) .*$/\1/')

sed -i -r "s|^(INCLUDE_PATH\s+=).*$|\1 sptk5:/usr/include:$CXX_INCLUDE_DIR|" sptk5.doxygen

echo Starting in $SPTKDIR
echo Generating HTML documentation
doxygen sptk5.doxygen
if [ $? != 0 ]; then
  echo Errors during HTML docs generation.
  echo Exiting..
  exit 1
fi

echo Compressing generated documentation
cd $SPTKDIR/docs/html
tar jcf ../sptk5_reference.html.tbz2 *

cd $SPTKDIR

echo Removing intermediate files
rm -f $SPTKDIR/html/*.html $SPTKDIR/html/*.png
mv $SPTKDIR/docs/html/* $SPTKDIR/html/
rm -rf $SPTKDIR/docs/latex $SPTKDIR/docs/html

