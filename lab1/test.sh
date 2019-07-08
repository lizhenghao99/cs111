#!/bin/bash

# NAME: 	Zhenghao Li
# EMAIL: 	lizhenghao99@g.ucla.edu
# ID:		704971934

# exec 2> /dev/null
# check for --rdonly error
touch testin.txt
chmod -r testin.txt
./simpsh --rdonly testin.txt 2> /dev/null
if [ $? -ne 1 ]
then
	echo "test 1 failed"
else
	echo "test 1 passed"
fi
rm -f testin.txt

# check for --rdonly --wronly --wronly --command
touch a.txt b.txt c.txt d.txt
flag2=0;
echo "this is file a.txt" > a.txt
./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --command 0 1 2 cat
diff a.txt b.txt > /dev/null
if [ $? -ne 0 ]
then
	flag2=1;
fi
diff c.txt d.txt >/dev/null
if [ $? -ne 0 ]
then 
	flag2=1;
fi
if [ $flag2 -eq 1 ]
then 
	echo "test 2 failed"
else
	echo "test 2 passed"
fi
rm -f a.txt b.txt c.txt d.txt

# check for --verbose --rdonly --wronly --wronly --command
touch a b c output result verbose
flag3=0;
echo "Some text in a file." > a
./simpsh --verbose --rdonly a --wronly b --wronly c --command 0 1 2 tr a-z A-Z > result
echo "SOME TEXT IN A FILE." > output
echo -e "--rdonly a\n--wronly b\n--wronly c\n--command 0 1 2 tr a-z A-Z " > verbose

diff b output 
if [ $? -ne 0 ]
then
	flag3=1;
fi
diff result verbose
if [ $? -ne 0 ]
then
	flag3=1;
fi
if [ $flag3 -eq 1 ]
then 
	echo "test 3 failed"
else
	echo "test 3 passed"
fi
rm -f a b c output result verbose

# check for --verbose --wait
./simpsh --creat --rdonly a --creat --wronly b --verbose --command 0 1 1 cat a c --verbose --wait > out
if [ $? -eq 1 ] && grep -q -- "exit 1 cat a c" out && wc -l < out | grep -q "3"
then 
	echo "test 4 passed"
else
	echo "test 4 failed"
fi
rm -f a b out


