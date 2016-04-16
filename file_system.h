#pragma once

#include <sys/types.h>

#define O_RDONLY (1 << 0)
#define O_WRONLY (1 << 1)
#define O_RDWR (1 << 2)
#define O_APPEND (1 << 3)
#define O_CREAT (1 << 4)
#define O_EXCL (1 << 5)
#define O_TRUNC (1 << 6)

typedef struct inode* DIR;

int open(const char* pathname, int flags);

void init_file_system();

void print_file_system();

ssize_t read(int fildes, void* buf, size_t nbyte);

ssize_t write(int fildes, const void* buf, size_t nbyte);

int mkdir(const char *path);

struct inode* readdir(DIR* dirp);

DIR* opendir(const char * path);

void closedir(DIR* dir);

int close(int fd);
