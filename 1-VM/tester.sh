#!/bin/bash

make
compiled=$?
if [[ $compiled != 0 ]]; then
	echo "does not compile"
	exit 1
fi

echo -n "Testing test1.txt : "
./vm test1.txt <<< '3' > output.txt
executed=$?
if [[ $executed !=  0 ]]; then
	echo "crashed"
	exit 1
fi
diff -w output.txt output1.txt &> /dev/null
correct=$?
if [[ $correct != 0 ]]; then
	echo "incorrect output"
else
	echo "pass"
fi


 
