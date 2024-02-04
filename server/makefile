all:server objects

SERVEROBJ = demoServer.o StdThread.o StdSqlite.o \
			MyString.o StdThreadPool.o DoubleLinkList.o \
			GlobalMessage.o StdTcp.o onLine.o \
			balanceBinarySearchTree.o doubleLinkListQueue.o doubleLinkList.o\
			LinkQueue.o
# 分立的obj
OBJS = demoServer StdThread StdSqlite \
		MyString StdThreadPool DoubleLinkList \
		GlobalMessage StdTcp onLine \
		balanceBinarySearchTree doubleLinkListQueue doubleLinkList\
		LinkQueue.o

LIBS = -lsqlite3 -g

server:$(SERVEROBJ) objects
	gcc $(SERVEROBJ) -o server.out $(LIBS)
	

objects:
	@$(foreach obj, $(OBJS), \
	@gcc -c $(obj).c -o $(obj).o -g;\
	#echo cc -c $(obj).c -o $(obj).o -g;\
	)\

clean:
	@rm -rf *.o server.out
