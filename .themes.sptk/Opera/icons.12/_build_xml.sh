echo "<icons size=\"combo\">"
for filename in *.png
do
    fname=`basename $filename | sed 's/.png//'`
    echo "  <icon name=\"$fname\" image=\"icons.12/$filename\" />"
done
echo "</icons>"