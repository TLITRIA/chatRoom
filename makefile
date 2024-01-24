# 自定义变量
OBJ1=DoubleLinkList.o GlobalMessage.o LinkQueue.o MyString.o StdSqlite.o StdTcp.o StdThread.o StdThreadPool.o server.o
TARGET1=server

# 使用$(TARGET) 必须要加 ‘$’ 符号

$(TARGET1):$(OBJ1)
	gcc $^ -o $@ -lsqlite3

DoubleLinkList.o:DoubleLinkList.c
	gcc -c $^ -o $@

GlobalMessage.o:GlobalMessage.c
	gcc -c $^ -o $@

LinkQueue.o:LinkQueue.c
	gcc -c $^ -o $@

MyString.o:MyString.c
	gcc -c $^ -o $@

StdSqlite.o:StdSqlite.c
	gcc -c $^ -o $@

StdTcp.o:StdTcp.c
	gcc -c $^ -o $@

StdThread.o:StdThread.c
	gcc -c $^ -o $@

StdThreadPool.o:StdThreadPool.c
	gcc -c $^ -o $@

server.o:server.c
	gcc -c $^ -o $@ 

# 自定义变量
OBJ2=GlobalMessage.o  StdTcp.o client.o StdThread.o DoubleLinkList.o
TARGET2=client

$(TARGET2):$(OBJ2)
	gcc $^ -o $@ 

DoubleLinkList.o:DoubleLinkList.c
	gcc -c $^ -o $@

GlobalMessage.o:GlobalMessage.c
	gcc -c $^ -o $@

StdTcp.o:StdTcp.c
	gcc -c $^ -o $@

StdThread.o:StdThread.c
	gcc -c $^ -o $@

client.o:client.c
	gcc -c $^ -o $@

clean:
	@rm -rf *.o $(TARGET)