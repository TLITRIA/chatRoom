all:server client objects


SERVEROBJ = demoServer.o common.o stdNet.o stdJson.o stdFile.o
CLIENTOBJ = demoClient.o common.o stdNet.o stdJson.o stdFile.o
#所有的文件各自生成.o
OBJS = demoServer demoClient common stdNet stdJson stdFile
LIBS=-ljson-c -g -lsqlite3
server:$(SERVEROBJ)
	gcc $(SERVEROBJ) -o server $(LIBS)

client:$(CLIENTOBJ)
	gcc $(CLIENTOBJ) -o client $(LIBS)

#stdJson.o:stdJson.c 
#	gcc -c stdJson.c -o stdJson.o -g

objects:
	@$(foreach obj, $(OBJS), \
	gcc -c $(obj).c -o $(obj).o -g;\
	#echo gcc -c $(obj).c -o $(obj).o -g;\
	)\


# 声明clean为伪目标
.PHONY:clean

clean:
	@rm -rf *.o server client
