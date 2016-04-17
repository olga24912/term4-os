#pragma once

#include <stdint.h>

struct inode {
    char* name; //name of file
    int capacity_level; // log(size of path)
    uint64_t size; // size of file
    struct inode* neighbor;
    struct inode* child;
    void* file_start; // start of file
    uint8_t is_dir; // true if it is directory and false if folder
};