if [ `uname` = Darwin ]; then
  echo gcc -dynamiclib -install_name lib/$1 -flat_namespace -undefined suppress -o lib/$1 $2
else
  echo gcc -shared -fPIC -o lib/$1 $2
fi
