#!/bin/bash

if [ $# -eq 0 ]; then
	echo "Compiling lex.c"
	gcc lex.c driver.c
else
	echo "Compiling $1"
	gcc $1 driver.c
fi
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
	.././a.out $f > output.txt
	executed=$?
	if [[ $executed !=  0 ]]; then
		echo "fail"
		exit 1
	else
		.././a.out $f | diff -w -B "$base.cmp" - &> /dev/null
		correct=$?
		if [[ $correct != 0 ]]; then
			echo "fail"
			exit 1
		else
			echo "pass"
		fi
	fi
done
