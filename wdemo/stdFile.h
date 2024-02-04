#ifndef __STD_FILE_H_
#define __STD_FILE_H_

/* 初始化一个例程文件 */
int file_init(char *file_path);

/* 创建接收文件, 并共享存储映射 */
char *file_new_recv(char *file_path, int length);

/* 获取文件大小 */
int file_get_size(char *file_path);


/* 共享存储映射 */
char *file_mmap_share(char *file_path, int file_share_size);









#endif // __STD_FILE_H_