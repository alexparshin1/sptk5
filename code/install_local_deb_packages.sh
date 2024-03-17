for package_file in $(ls sptk-*.deb); do
    PACKAGES="$PACKAGES ./$package_file"
done

apt install $PACKAGES
rm -f install_manifest_*
