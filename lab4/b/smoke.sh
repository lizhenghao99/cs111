#!/bin/bash

# NAME:		Zhenghao Li
# EMAIL:	lizhenghao99@g.ucla.edu
# ID:		704971934

{ echo "STOP"; sleep 2; echo "START"; sleep 2; echo "OFF"; } | ./lab4b --log=out

flag=0

if [ $? -ne 0 ]
then 
	echo "bad rc"
	flag=1
fi

grep -q "OFF" out
if [ $? -ne 0 ]
then
	echo "bad OFF"
	flag=1
fi

grep -q "START" out
if [ $? -ne 0 ]
then
	echo "bad START"
	flag=1
fi

grep -q "SHUTDOWN" out
if [ $? -ne 0 ]
then 
	echo "bad SHUTDOWN"
	flag=1
fi

if [ $flag -ne 1 ]
then 
	echo "smoke test passed"
else
	echo "smoke test failed"
fi

rm -f out
