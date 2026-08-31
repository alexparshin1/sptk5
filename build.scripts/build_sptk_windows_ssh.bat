SPTK_VERSION=$(cat SPTK_VERSION)
ssh alexe@windev2 <<EOF
    cd \workspace\sptk5\build.scripts
    git reset --hard
    git pull
    git checkout $SPTK_VERSION
    build_sptk_windows.bat
EOF
