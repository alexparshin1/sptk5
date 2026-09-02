#!/bin/sh

CWD=$(pwd)
THEME=$1
shift
cd $THEME
tar cf $CWD/$THEME.tar $@
