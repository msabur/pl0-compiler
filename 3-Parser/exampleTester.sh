#!/bin/bash

if [[ $1 == "-v" ]]; then
	show_diffs=true
else
	show_diffs=false
fi

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
		if [[ $show_diffs == true ]]; then
			echo "Diff in the form < (correct output) > (our output):"
			echo "$output"
		fi
	else
		echo "Pass"
	fi
done
