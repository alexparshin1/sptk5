npm run build

[ -e /var/www/html/sptk/static ] && rm -rf /var/www/html/sptk/static

echo Install build to theater..
rsync -a ./build/* theater:/var/www/html/sptk/
rsync -a ./build/.htaccess theater:/var/www/html/sptk/
scp -q extra_files/* theater:/var/www/html/sptk/
