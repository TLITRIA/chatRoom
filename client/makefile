all:client objects

CLIENTOBJ = demoClient.o StdTcp.o DoubleLinkList.o StdThread.o

OBJ = demoClient StdTcp DoubleLinkList StdThread

LIBS = -g

client:$(CLIENTOBJ) objects
	gcc $(CLIENTOBJ) -o client.out $(LIBS)

objects:
	@$(foreach obj, $(OBJS), \
	@gcc -c $(obj).c -o $(obj).o -g;\
	#echo cc -c $(obj).c -o $(obj).o -g;\
	)\

clean:
	@rm -rf *.o client.out
