CFLAGS= -L../libsnp -Wl,-rpath=../libsnp

all: client server

client: client.o
	gcc -o client client.o ${CFLAGS} -lsnp

client.o: client.c
	gcc -c client.c

server: server.o
	gcc -o server server.o ${CFLAGS} -lsnp

server.o: server.c
	gcc -c server.c

clean:
	rm -rf *.log *.o
