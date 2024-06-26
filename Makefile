CC=gcc
CFLAGS=-g
TARGET:test.exe
LIBS : -lpthread -L ./CommandParser -lcli
OBJS=gluethread/glthread.o graph.o topologies.o CommandParser/libcli.a net.o utils.o nwcli.o comm.o Layer2/layer2.o Layer3/layer3.o Layer2/l2switch.o pkt_dump.o

test.exe:testapp.o ${OBJS} 
	${CC} ${CFLAGS} testapp.o ${OBJS} ${LIBS} -o test.exe 
testapp.o:testapp.c
	${CC} ${CFLAGS} -c testapp.c -o testapp.o
gluethread/glthread.o:gluethread/glthread.c
	${CC} ${CFLAGS} -c -I gluethread gluethread/glthread.c -o gluethread/glthread.o
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
comm.o:comm.c
	${CC} ${CFLAGD} -c comm.c -o comm.o
CommandParser/libcli.a:
	(cd CommandParser; make)
Layer2/layer2.o:Layer2/layer2.c
	${CC} ${CFLAGS} -c -I . Layer2/layer2.c -o Layer2/layer2.o
Layer2/l2switch.o:Layer2/l2switch.c
	${CC} ${CFLAGS} -c -I . Layer2/l2switch.c -o Layer2/l2switch.o
Layer3/layer3.o:Layer3/layer3.c
	${CC} ${CFLAGS} -c -I . Layer3/layer3.c -o Layer3/layer3.o
pkt_dump.o:pkt_dump.c
	${CC} ${CFLAGS} -c pkt_dump.c -o pkt_dump.o
clean:
	rm *.o
	rm gluethread/glthread.o
	rm *exe
	(cd CommandParser; make clean)
	rm Layer2/*.o
	rm Layer3/*.o
