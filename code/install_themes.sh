#!/bin/sh

function themeInstall() {
    local themeName=$1
    echo "  theme: $themeName"
    mkdir -p $install_prefix/$themeName
    cp -r themes/$themeName/* $install_prefix/$themeName/
    rm -rf $install_prefix/$themeName/.svn $install_prefix/$themeName/dialog_icons/.svn
}

# SPTK headers installation

install_prefix=$1

if [ "$install_prefix" = "" ]; then
    install_prefix="/usr/local"
fi

install_prefix=$install_prefix/share/sptk

echo Installing themes into $install_prefix

for themeName in `ls themes` 
do
    if [ -d themes/$themeName ] && [ ! $themeName = ".svn" ]; then
	themeInstall $themeName
    fi
done
