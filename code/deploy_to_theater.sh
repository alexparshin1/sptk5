#!/bin/bash

pwd | grep sptk5

if [ $? == 0 ]; then
    ROOT=workspace/sptk5/code
else
    ROOT=workspace/protis
fi

ssh theater <<EOF
    cd $ROOT && git reset --hard && git pull && make -j4 install
EOF
