echo "<icons size=\"small\">"
for filename in *.png
do
    fname=`basename $filename | sed 's/.png//'`
    echo "<icon name=\"$fname\" image=\"icons.16/$filename\" />"
done
echo "</icons>"