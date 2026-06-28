echo "<icons size=\"large\">"
for filename in *.png
do
    fname=`basename $filename | sed 's/.png//'`
    echo "  <icon name=\"$fname\" image=\"icons.22/$filename\" />"
done
echo "</icons>"