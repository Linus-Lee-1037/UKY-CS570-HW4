# 定义编译器和编译选项
CC = gcc
CXX = g++
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g

# 目标
all: client server

# 生成客户端可执行文件
client: client.o ssnfs_clnt.o ssnfs_xdr.o
	$(CXX) $(CXXFLAGS) -o client client.o ssnfs_clnt.o ssnfs_xdr.o

# 生成服务器可执行文件
server: server.o ssnfs_svc.o ssnfs_xdr.o
	$(CXX) $(CXXFLAGS) -o server server.o ssnfs_svc.o ssnfs_xdr.o

# 编译客户端源文件（C++）
client.o: ssnfs.h client.cpp
	$(CXX) $(CXXFLAGS) -c client.cpp

# 编译服务器源文件（C++）
server.o: ssnfs.h server.cpp
	$(CXX) $(CXXFLAGS) -c server.cpp

# 编译 ssnfs_svc.c
ssnfs_svc.o: ssnfs_svc.c ssnfs.h
	$(CC) $(CFLAGS) -c ssnfs_svc.c

# 编译 ssnfs_clnt.c
ssnfs_clnt.o: ssnfs_clnt.c ssnfs.h
	$(CC) $(CFLAGS) -c ssnfs_clnt.c

# 编译 ssnfs_xdr.c
ssnfs_xdr.o: ssnfs_xdr.c ssnfs.h
	$(CC) $(CFLAGS) -c ssnfs_xdr.c

# 清理生成的文件
clean:
	rm -f *.o client server ssnfs_svc.c ssnfs_clnt.c ssnfs_xdr.c ssnfs.h

	@if [ -e disk.dat ]; then \
	    echo "Deleting disk.dat"; \
	    rm -f disk.dat; \
	fi

	@if [ -e files.dat ]; then \
	    echo "Deleting files.dat"; \
	    rm -f files.dat; \
	fi
