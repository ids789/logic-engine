CC=gcc
CFLAGS=-Wall
INCLUDE=-L/usr/lib/
INSTALL_PATH=/usr/bin
APP_NAME=logic-engine

logic-engine: main.o common.o knx.o
	$(CC) -o $(APP_NAME) main.o common.o knx.o -lchibi-scheme -leibclient

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c common.h

knx.o: knx.c common.h
	$(CC) $(CFLAGS) -c knx.c common.h

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

install:
	cp $(APP_NAME) $(INSTALL_PATH)

clean:
	rm *.o *.gch logic-engine
