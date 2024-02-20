OBJS = $(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET = libSoChatRoom.so

$(TARGET):$(OBJS)
	$(CC) -shared -fPIC $^ -o $@ -g
	cp $(TARGET) /usr/lib

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) /usr/lib/$(TARGET)









