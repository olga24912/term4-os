#pragma once

#include <sys/types.h>

// flags for function open folder
// For correct work need to ude exactly one of flags O_RDONLY O_WRONLY O_RDWR
#define O_RDONLY (1 << 0)
#define O_WRONLY (1 << 1)
#define O_RDWR (1 << 2)
#define O_APPEND (1 << 3)
#define O_CREAT (1 << 4)
#define O_EXCL (1 << 5)
#define O_TRUNC (1 << 6)

//used in readdir, opendir, closedir
typedef struct inode* DIR;

//open file
int open(const char* pathname, int flags);

//function for initialization file system
void init_file_system();

//print file system tree in some way
void print_file_system();

//read from file to buf n bytes, return count of read bytes
ssize_t read(int fildes, void* buf, size_t nbyte);

//write from buf to filr n bytes return count of write bytes
ssize_t write(int fildes, const void* buf, size_t nbyte);

//create dir
int mkdir(const char *path);

//return some info about subdirs, firstly need call opendir
struct inode* readdir(DIR* dirp);

DIR* opendir(const char * path);

//need call after work with dir
void closedir(DIR* dir);

int close(int fd);
