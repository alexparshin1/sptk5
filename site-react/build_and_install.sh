npm run build

echo Install build to theater..
# Dropped first, so that renamed bundles from earlier builds don't pile up there
ssh theater 'rm -rf /var/www/html/sptk/static'
rsync -a ./build/* theater:/var/www/html/sptk/
rsync -a ./build/.htaccess theater:/var/www/html/sptk/
scp -q extra_files/* theater:/var/www/html/sptk/
