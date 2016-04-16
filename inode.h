#pragma once

#include <stdint.h>

struct inode {
    char* name;
    int capacity_level;
    uint64_t size;
    struct inode* neighbor;
    struct inode* child;
    void* file_start;
    uint8_t is_dir;
};