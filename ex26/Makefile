CFLAGS=-Wall -g -DNDEBUG
EX=logfind

all:
	make ${EX}
	./logfind clear clean
	./logfind clear clean -o
	./logfind sync clear clean
	./logfind sync clear clean -o

clean:
	rm -f ${EX} *.o
