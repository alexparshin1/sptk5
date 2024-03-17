PACKAGES=""
for package_file in $(ls sptk-*.deb); do
    PACKAGES="$PACKAGES ./$package_file"
done

if [ ! "$PACKAGES" = "" ]; then
    apt -y install $PACKAGES
    rc=$?
    rm -f install_manifest_*
    exit $rc
fi

for package_file in $(ls sptk-*.rpm); do
    PACKAGES="$PACKAGES ./$package_file"
done

if [ ! "$PACKAGES" = "" ]; then
    yum -y install $PACKAGES
    rc=$?
    rm -f install_manifest_*
    exit $rc
fi
