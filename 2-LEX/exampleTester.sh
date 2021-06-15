#!/bin/bash

make
compiled=$?
if [[ $compiled != 0 ]]; then
	echo "Does not compile"
	exit 1
fi

echo "Compiles"

echo -n "Testing Tokens : "
./lex lex_example1.txt > output.txt
executed=$?
if [[ $executed !=  0 ]]; then
	echo "Crashed"
	exit 1
else
	diff -w -B output.txt lexout1.txt &> /dev/null
	correct=$?
	if [[ $correct != 0 ]]; then
		echo "Fail"
		exit 1
	else
		echo "OK"
	fi
fi


echo -n "Testing Errors : "

echo -n "lex_example2.txt : "
./lex lex_example2.txt > output.txt
executed=$?
if [[ $executed !=  0 ]]; then
	echo "Crashed"
else
	diff -w -B output.txt lexout2.txt &> /dev/null
	correct=$?
	if [[ $correct != 0 ]]; then
		echo "Fail"
	else
		echo "OK"
	fi
fi
