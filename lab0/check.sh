# !/bin/bash

# NAME: 	Zhenghao Li
# EMAIL: 	lizhenghao99@g.ucla.edu
# ID: 		704971934

# check for --input
echo "input text" > in.txt
./lab0 --input in.txt &>/dev/null
if [ $? -ne 0 ]
then
	echo "-i test failed" >> error.txt
else
	echo "-i test passed"
fi
rm -f in.txt

# check for --output
echo "output text" | ./lab0 --output out.txt
if [ $? -ne 0 ]
then
	echo "-o test failed" >> error.txt
else
	echo "-o test passed"
fi
rm -f out.txt

# check for --output --input
echo "inputoutput text" > in.txt
./lab0 --output out.txt --input in.txt
if [ $? -ne 0 ]
then 
	echo "-o -i test failed" >> error.txt
else
	echo "-o -i test passed"
fi
rm -f in.txt out.txt

# suppress error
exec 2> /dev/null

# check for unrecognized option
./lab0 --randomOption &>/dev/null
if [ $? -ne 1 ]
then 
	echo "invalid option test failed" >> error.txt
else
	echo "invalid option test passed"
fi

# check for --segfault
./lab0 --segfault &>/dev/null
if [ $? -ne 139 ]
then 
	echo "-s test failed" >> error.txt
else
	echo "-s test passed" 
fi

# check for --catch --segfault
./lab0 --catch --segfault &>/dev/null
if [ $? -ne 4 ]
then 
	echo "-c -s test failed" >> error.txt
else
	echo "-c -s test passed" 
fi

# check for --catch --dump-core --segfault
./lab0 --catch --dump-core --segfault &>/dev/null
if [ $? -ne 139 ]
then 
	echo "-c -d -s test failed" >> error.txt
else
	echo "-c -d -s test passed"
fi

# result
if [ -e error.txt ]
then
	cat error.txt
	echo "smoke test FAILED!!!"
	rm -f error.txt
else
	echo "smoke test passed!"
fi
