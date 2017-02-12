CFLAGS=-Wall -g -DNDEBUG
EX=logfind

all:
	make ${EX}
	./logfind clear
	./logfind clear clean
	./logfind clear clean -o
	./logfind complete clear clean
	./logfind complete clear clean -o

clean:
	rm -f ${EX} *.o
