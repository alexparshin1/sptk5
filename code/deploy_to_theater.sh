#!/bin/bash

ssh theater <<EOF
    cd workspace/sptk5/code && git pull -f && make -j4 install
EOF