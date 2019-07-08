# NAME:		Zhenghao Li
# EMAIL:	lizhenghao99@g.ucla.edu
# ID:		704971934


default: lab3b

lab3b: lab3b.py lab3b.sh
	@chmod +x lab3b.sh
	@ln -f lab3b.sh lab3b
	@echo "Executable 'lab3b' created"

dist: lab3b.py lab3b.sh Makefile README
	tar -czf lab3b-704971934.tar.gz lab3b.py lab3b.sh Makefile README

clean:
	rm -f lab3b
	rm -f lab3b-704971934.tar.gz
