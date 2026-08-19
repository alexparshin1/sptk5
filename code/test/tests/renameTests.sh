for file in $(find -name '*.cpp' | grep -v Test)
do
    fname=$(echo $file | sed -re 's|\.cpp|Tests.cpp|')
    git mv $file $fname
done
