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
	# .././a.out $f | diff -w -B "$base.cmp" - &> /dev/null
	output=$(.././a.out $f | diff -w -B "$base.cmp" -)
	correct=$?
	if [[ $correct != 0 ]]; then
		echo "Fail"
		echo "Here's the diff in the form < (correct output) > (our output):"
		echo "$output"
	else
		echo "Pass"
	fi
done
