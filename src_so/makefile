OBJS = $(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET = libSoChatRoom.so

LIBS=-lsqlite3

$(TARGET):$(OBJS)
	$(CC) -g -shared -fPIC $^ $(LIBS) -o $@
	cp $(TARGET) /usr/lib

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) /usr/lib/$(TARGET)









