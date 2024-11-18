all: client server
client : client.o ssnfs_clnt.o  ssnfs_xdr.o  
	gcc -o client client.o ssnfs_clnt.o ssnfs_xdr.o -I/usr/include/tirpc -ltirpc
server: ssnfs_xdr.o  ssnfs_svc.o server.o
	gcc -o server server.o ssnfs_svc.o ssnfs_xdr.o -I/usr/include/tirpc -ltirpc
client.o: ssnfs.h client.c 
	gcc -c client.c -I/usr/include/tirpc -ltirpc
server.o: ssnfs.h server.c
	gcc -c server.c  -I/usr/include/tirpc -ltirpc
ssnfs_svc.o: ssnfs.h ssnfs_svc.c 
	gcc -c ssnfs_svc.c -I/usr/include/tirpc -ltirpc
ssnfs_clnt.o: ssnfs_clnt.c ssnfs.h
	gcc -c ssnfs_clnt.c  -I/usr/include/tirpc -ltirpc
ssnfs_xdr.o: ssnfs_xdr.c ssnfs.h
	gcc -c ssnfs_xdr.c -I/usr/include/tirpc -ltirpc

clean :
	rm  *.o *~

 