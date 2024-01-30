all:client server

# 客户端程序
client:client.o StdTcp.o StdThread.o GlobalMessage.o DoubleLinkList.o 
	gcc client.o StdTcp.o StdThread.o GlobalMessage.o DoubleLinkList.o -o client -lsqlite3

# 服务端程序
server:server.o StdThread.o StdSqlite.o MyString.o StdThreadPool.o DoubleLinkList.o GlobalMessage.o StdTcp.o LinkQueue.o balanceBinarySearchTree.o doubleLinkList.o doubleLinkListQueue.o onLine.o
	gcc server.o StdThread.o StdSqlite.o MyString.o StdThreadPool.o DoubleLinkList.o GlobalMessage.o StdTcp.o LinkQueue.o balanceBinarySearchTree.o doubleLinkList.o doubleLinkListQueue.o onLine.o -o server -lsqlite3

server.o:server.c
	gcc -c server.c -o server.o

client.o:client.c
	gcc -c client.c -o client.o

LinkQueue.o:LinkQueue.c
	gcc -c LinkQueue.c -o LinkQueue.o

StdSqlite.o:StdSqlite.c
	gcc -c StdSqlite.c -o StdSqlite.o


StdThread.o:StdThread.c
	gcc -c StdThread.c -o StdThread.o

StdThreadPool.o:StdThreadPool.c 
	gcc -c StdThreadPool.c -o StdThreadPool.o

MyString.o:MyString.c
	gcc -c MyString.c -o MyString.o

DoubleLinkList.o:DoubleLinkList.c
	gcc -c DoubleLinkList.c -o DoubleLinkList.o

GlobalMessage.o:GlobalMessage.c
	gcc -c GlobalMessage.c -o GlobalMessage.o

StdTcp.o:StdTcp.c
	gcc -c StdTcp.c -o StdTcp.o

balanceBinarySearchTree.o:balanceBinarySearchTree.c
	gcc -c balanceBinarySearchTree.c -o balanceBinarySearchTree.o

doubleLinkList.o:doubleLinkList.c
	gcc -c doubleLinkList.c -o doubleLinkList.o

doubleLinkListQueue.o:doubleLinkListQueue.c
	gcc -c doubleLinkListQueue.c -o doubleLinkListQueue.o

onLine.o:onLine.c
	gcc -c onLine.c -o onLine.o

clean:
	@rm -rf *.o client server
