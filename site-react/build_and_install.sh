npm run build

[ -e /var/www/html/sptk/static ] && rm -rf /var/www/html/sptk/static

rsync -av ./build/* /var/www/html/sptk/