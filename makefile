OBJ=$(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET=main

LDFLAGS=-L./datastruct 
LIBS=-lDoubleLinkList -lpthread -lsqlite

$(TARGET):$(OBJ)
	$(CC) $^ -o $@

%.o:%.c
	$(CC) -c $^ -o $@


#伪目标  / 伪文件
.PHONY:clean

clean:
	@$(RM) *.o $(TARGET)