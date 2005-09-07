#! /bin/sh

for i in $@; do
  test -f ${i%%.*}.cal && { cat ${i%%.*}.cal; echo; echo; echo; }
  test -f $i && { cat $i; echo ""; }
done | enscript -ffixed14 -pexample.ps
