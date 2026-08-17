npm run build

echo Install build to theater..
# Dropped first, so that renamed bundles from earlier builds don't pile up there
ssh theater 'rm -rf /var/www/html/sptk/static'
# The PHP endpoints go first and the build second, so that on any name they share
# the generated file wins. Copies of logo192.png and logo512.png once sat in
# extra_files and, being copied last, silently replaced the deployed icons.
scp -q extra_files/* theater:/var/www/html/sptk/
rsync -a ./build/* theater:/var/www/html/sptk/
rsync -a ./build/.htaccess theater:/var/www/html/sptk/
