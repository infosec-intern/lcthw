CFLAGS=-Wall -g
EX=logfind

all:
	make ${EX}

clean:
	rm -f ${EX} *.o
