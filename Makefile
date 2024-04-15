CC=gcc
CFLAGS=-g
TARGET:test.exe
LIBS : -lpthread -L ./CommandParser -lcli
OBJS=gluethread/glthread.o graph.o topologies.o CommandParser/libcli.a comm.o net.o utils.o nwcli.o

test.exe:testapp.o ${OBJS} 
	${CC} ${CFLAGS} testapp.o ${OBJS} ${LIBS} -o test.exe 
testapp.o:testapp.c
	${CC} ${CFLAGS} -c testapp.c -o testapp.o
gluethread/glthread.o:gluethread/glthread.c
	${CC} ${CFLAGS} -c gluethread/glthread.c -o gluethread/glthread.o
graph.o:graph.c
	${CC} ${CFLAGS} -c graph.c -o graph.o
topologies.o:topologies.c
	${CC} ${CFLAGS} -c topologies.c -o topologies.o
comm.o:comm.c
	${CC} ${CFLAGS} -c comm.c -o comm.o
net.o:net.c
	${CC} ${CFLAGS} -c net.c -o net.o
utils.o:utils.c 
	${CC} ${CFLAGS} -c utils.c -o utils.o
nwcli.o:nwcli.c
	${CC} ${CFLAGD} -c nwcli.c -o nwcli.o
CommandParser/libcli.a:
	(cd CommandParser; make)
clean:
	rm *.o
	rm gluethread/glthread.o
	rm *exe
	(cd CommandParser; make clean)
