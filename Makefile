all: client server

client: client.o ssnfs_clnt.o ssnfs_xdr.o
	gcc-14 -o client client.o ssnfs_clnt.o ssnfs_xdr.o

server: server.o ssnfs_svc.o ssnfs_xdr.o
	gcc-14 -o server server.o ssnfs_svc.o ssnfs_xdr.o

client.o: ssnfs.h client.c
	gcc-14 -c client.c

server.o: ssnfs.h server.c
	gcc-14 -c server.c

ssnfs_svc.o: ssnfs_svc.c ssnfs.h
	gcc-14 -c ssnfs_svc.c

ssnfs_clnt.o: ssnfs_clnt.c ssnfs.h
	gcc-14 -c ssnfs_clnt.c

ssnfs_xdr.o: ssnfs_xdr.c ssnfs.h
	gcc-14 -c ssnfs_xdr.c

clean:
	rm -f *.o client server
