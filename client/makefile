OBJS = $(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET = client.out


LIBS=-ljson-c -lsqlite3 -lSoChatRoom

$(TARGET):$(OBJS)
	$(CC) -g $^  $(LIBS) -o $@ 

%.o:%.c
	$(CC) -g -c $^ -o $@

.PHONY: clean

clean:
	@$(RM) *.o $(TARGET) *.out

show:
	@echo $(RM)
	@echo $(CC)
	@echo $(CXX)
	@echo $(MAKE)
	@echo $(wildcard ./*.c)
	@echo $(patsubst %.c, %.o, $(wildcard ./*.c))