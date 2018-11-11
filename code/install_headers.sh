#!/bin/sh

# SPTK headers installation

install_prefix=$1

if [ "$install_prefix" = "" ]; then
	install_prefix="/usr/local"
fi

echo Prefix $install_prefix

if [ -d $install_prefix/include/sptk3/xml ]; then
	echo SPTK XML include directory exists, using it.
else
	echo Creating SPTK XML include files directory.
	mkdir -p $install_prefix/include/sptk3/xml
fi

if [ -d $install_prefix/include/sptk3/excel ]; then
	echo SPTK Excel include directory exists, using it.
else
	echo Creating SPTK XML include files directory.
	mkdir -p $install_prefix/include/sptk3/excel
fi

echo Installing SPTK include files into $install_prefix/include/sptk

cp sptk3/*.h sptk3/c*  $install_prefix/include/sptk3
cp sptk3/xml/*.h $install_prefix/include/sptk3/xml
cp sptk3/excel/*.h $install_prefix/include/sptk3/excel
chmod -R a+r $install_prefix/include/sptk3
