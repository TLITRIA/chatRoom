# 当前目录下所有.c文件各自生成同名可执行文件
all:objects outs sqlite3_test

OBJS=json_test\
	realloc_test\
	stol_test\
	tcgetattr_test\
	signal_test\
	sqlite3_test\
	stdSqlite3\
	mmap_test\
	string_test\
	point_struct_text\
	cal_text\
	colorprint_test\

INDIVIDUAL=json_test\
	realloc_test\
	stol_test\
	tcgetattr_test\
	signal_test\
	mmap_test\
	string_test\
	point_struct_text\
	cal_text\
	colorprint_test\
	

LIBS=-ljson-c -g -lsqlite3

objects:
	@$(foreach obj, $(OBJS), \
	gcc -c $(obj).c -o $(obj).o;\
	)\

outs:objects
	@$(foreach obj, $(INDIVIDUAL), \
	gcc $(obj).o -o $(obj).out $(LIBS);\
	)\

sqlite3_test:objects
	@gcc sqlite3_test.o stdSqlite3.o -o sqlite3_test.out $(LIBS)


clean:
	@rm -rf *.out *.o