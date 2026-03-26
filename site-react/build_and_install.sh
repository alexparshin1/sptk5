npm run build

[ -e /var/www/html/sptk/static ] && rm -rf /var/www/html/sptk/static

rsync -av ./build/* theater:/var/www/html/sptk/
scp extra_files/* theater:/var/www/html/sptk/
