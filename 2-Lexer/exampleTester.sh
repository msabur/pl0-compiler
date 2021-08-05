#!/bin/bash

make
compiled=$?
if [[ $compiled != 0 ]]; then
	echo "does not compile"
	exit 1
fi

echo "Compiles"

cd tests

for f in *.in
do
	base="${f%.*}" # the filename without the extension at the end
	echo -n "Testing $f : "
	.././a.out $f | diff -w -B "$base.cmp" - &> /dev/null
	correct=$?
	if [[ $correct != 0 ]]; then
		echo "fail"
	else
		echo "pass"
	fi
done
