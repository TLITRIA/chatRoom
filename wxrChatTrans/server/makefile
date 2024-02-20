OBJS = $(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET = server.out

LDFALGS = -L../src_so

LIBS=-ljson-c -lsqlite3 -lSoChatRoom

$(TARGET):$(OBJS)
	$(CC) -g $^ -o $@ $(LIBS)

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