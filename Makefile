CFLAGS=-Wall -g
EX=

all:
	make ${EX}

ec:
	gcc ${CFLAGS} -o ec extracredit.c

broken:
	gcc ${CFLAGS} -o broken broken.c

clean:
	rm -f ${EX} broken ec
