#include <stdbool.h>
#include <stdint.h>
#include "initramfs.h"
#include "SLAB_allocator.h"
#include "file_system.h"

void* current_addr;

uint32_t str_to_int(char* s, size_t len) {
    uint32_t x = 0;
    for (size_t i = 0; i < len; ++i) {
        x = x * 16 ;
        if (s[i] >= '0' && s[i] <= '9') {
            x += s[i] - '0';
        } else if (s[i] >= 'a' && s[i] <= 'z'){
            x += s[i] - 'a' + 10;
        } else {
            x += s[i] - 'A' + 10;
        }
    }

    return x;
}

int add_one_file() {
    if ((uint64_t)current_addr % 4 == 0) current_addr -= 4;
    current_addr += (4 - (((uint64_t)current_addr)%4));

    if (current_addr >= init_ref_end) {
        return 0;
    }

    struct cpio_header* header = (struct cpio_header*)VA(current_addr);
    current_addr += sizeof(struct cpio_header);

    uint32_t name_len = str_to_int(header->namesize, 8);

    char* name = malloc_small(name_len + 1);
    for (uint32_t i = 0; i < name_len; ++i) {
        name[i] = ((char*)VA(current_addr))[i];
    }
    name[name_len] = 0;
    current_addr += name_len;

    if (name_len == 11 && strncmp(name, END_OF_ARCHIVE, 11) == 0) {
        return 0;
    }

    while (name_len > 0 && name[0] != '/') {
        ++name;
        --name_len;
    }

    uint32_t mode = str_to_int(header->mode, 8);

    if (S_ISDIR(mode)) {
        mkdir(name);
    } else if (S_ISREG(mode)) {
        int file = open(name, O_WRONLY|O_CREAT|O_EXCL);

        uint32_t filesize = str_to_int(header->filesize, 8);

        if ((uint64_t)current_addr % 4 == 0) current_addr -= 4;
        current_addr += 4 - ((uint64_t)current_addr)%4;

        write(file, VA(current_addr), filesize);

        current_addr += filesize;
    }
    return 1;
}

void initramfs_to_fs() {
    printf("START add to initranfs\n");
    current_addr = init_ref;
    while (add_one_file());
    printf("FINISH add to initranfs\n");
}