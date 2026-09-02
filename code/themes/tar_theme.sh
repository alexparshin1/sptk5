#!/bin/sh

CWD=$(pwd)
THEME=$1
THEME_DIR=$2
shift
shift
cd $THEME_DIR
tar cf $CWD/$THEME.tar $@
