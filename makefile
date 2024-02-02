all:client server

# 客户端程序
client:client.o StdTcp.o StdThread.o GlobalMessage.o DoubleLinkList.o 
	gcc client.o StdTcp.o StdThread.o GlobalMessage.o DoubleLinkList.o -o client -lsqlite3 -g

# 服务端程序
server:server.o StdThread.o StdSqlite.o MyString.o StdThreadPool.o DoubleLinkList.o GlobalMessage.o StdTcp.o LinkQueue.o balanceBinarySearchTree.o doubleLinkList.o doubleLinkListQueue.o onLine.o
	gcc server.o StdThread.o StdSqlite.o MyString.o StdThreadPool.o DoubleLinkList.o GlobalMessage.o StdTcp.o LinkQueue.o balanceBinarySearchTree.o doubleLinkList.o doubleLinkListQueue.o onLine.o -o server -lsqlite3 -g

server.o:server.c
	gcc -c server.c -o server.o -g

client.o:client.c
	gcc -c client.c -o client.o -g

LinkQueue.o:LinkQueue.c
	gcc -c LinkQueue.c -o LinkQueue.o -g

StdSqlite.o:StdSqlite.c
	gcc -c StdSqlite.c -o StdSqlite.o -g


StdThread.o:StdThread.c
	gcc -c StdThread.c -o StdThread.o -g

StdThreadPool.o:StdThreadPool.c 
	gcc -c StdThreadPool.c -o StdThreadPool.o -g

MyString.o:MyString.c
	gcc -c MyString.c -o MyString.o -g

DoubleLinkList.o:DoubleLinkList.c
	gcc -c DoubleLinkList.c -o DoubleLinkList.o -g

GlobalMessage.o:GlobalMessage.c
	gcc -c GlobalMessage.c -o GlobalMessage.o -g

StdTcp.o:StdTcp.c
	gcc -c StdTcp.c -o StdTcp.o -g

balanceBinarySearchTree.o:balanceBinarySearchTree.c
	gcc -c balanceBinarySearchTree.c -o balanceBinarySearchTree.o -g

doubleLinkList.o:doubleLinkList.c
	gcc -c doubleLinkList.c -o doubleLinkList.o -g

doubleLinkListQueue.o:doubleLinkListQueue.c
	gcc -c doubleLinkListQueue.c -o doubleLinkListQueue.o -g

onLine.o:onLine.c
	gcc -c onLine.c -o onLine.o -g

clean:
	@rm -rf *.o client server
