for package_file in $(ls sptk-*.deb); do
    PACKAGE_NAME=$(echo $package_file | sed -re 's/^(.*)_5.*$/\1/') #
    PACKAGES="$PACKAGES $PACKAGE_NAME"
done

apt remove $PACKAGES
