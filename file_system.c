#include <stddef.h>
#include "file_system.h"
#include "inode.h"
#include "SLAB_allocator.h"

#define MAX_FILE_DIS_COUNT (1 << 16)

static struct inode* root;
struct slabctl** inode_slab;

struct file_descriptor {
    struct inode* node;
    uint64_t current_position;
    int flags;
    int is_free;
};

struct file_descriptor file_descriptors[MAX_FILE_DIS_COUNT];

void init_file_system() {
    for (int i = 0; i < MAX_FILE_DIS_COUNT; ++i) {
        file_descriptors[i].is_free = 1;
    }
    inode_slab = create_slab_system(sizeof(struct inode), 1);

    root = allocate_block_in_slab_system(inode_slab);
    root->neighbor = NULL;
    root->child = NULL;
    root->is_dir = 1;
    root->name = "";
}

int find_first_free() {
    int i = 0;
    for (; i < MAX_FILE_DIS_COUNT && file_descriptors[i].is_free == 0; ++i);
    if (i < MAX_FILE_DIS_COUNT) {
        return i;
    }
    return -1;
}

void next_dir(const char *pathname, size_t *pos, size_t *new_len) {
    *pos += *new_len;

    if (pathname[*pos] == '/' ) {
        ++*pos;
    }

    *new_len = 0;
    while (pathname[*pos + *new_len] != 0) {
        if (pathname[*pos + *new_len] == '/') {
            break;
        }
        ++(*new_len);
    }
}

struct inode* create_new_file(const char *name) {
    struct inode* node = allocate_block_in_slab_system(inode_slab);


    size_t len = strlen(name);
    node->is_dir = 0;
    node->name = malloc_small((unsigned int) len + 1);

    for (size_t i = 0; i < len; ++i) {
        node->name[i] = name[i];
    }
    node->name[len] = 0;

    node->capacity_level = 0;
    node->file_start = get_page0(node->capacity_level);
    node->size = 0;

    node->child = NULL;
    node->neighbor = NULL;

    return node;
}

struct inode* create_new_dir(const char *name) {
    struct inode* node = allocate_block_in_slab_system(inode_slab);

    size_t len = strlen(name);
    node->is_dir = 1;
    node->name = malloc_small((unsigned int) len + 1);

    for (size_t i = 0; i < len; ++i) {
        node->name[i] = name[i];
    }
    node->name[len] = 0;

    node->capacity_level = 0;
    node->file_start = NULL;
    node->size = 0;

    node->child = NULL;
    node->neighbor = NULL;

    return node;
}

struct inode* find_file(const char* pathname, int force, struct inode*(*create_new)(const char*)) {
    size_t pos = 0;
    size_t len = 0;
    next_dir(pathname, &pos, &len);

    struct inode* node = root;

    while  (len != 0) {
        if (!node->is_dir) {
            return NULL;
        }
        struct inode* child = node->child;
        while (child != NULL && (strlen(child->name) != len || (strncmp(child->name, pathname + pos, len) != 0))) {
            child = child->neighbor;
        }

        if (!child) {
            if (!force) {
                return NULL;
            } else {
                if (pathname[pos + len] == 0) {
                    child = create_new(pathname + pos);
                    child->neighbor = node->child;
                    node->child = child;
                    return child;
                } else {
                    return NULL;
                }
            }
        }

        node = child;

        next_dir(pathname, &pos, &len);
    }

    if (force == 2) {
        return NULL;
    }
    return node;
}

DIR* opendir_inode(struct inode * node) {
    if (node == NULL || !node->is_dir) {
        return NULL;
    }
    DIR* res = malloc_small(sizeof(DIR));
    *res = node->child;
    return res;
}

void print_tree(struct inode* v, int h) {
    for (int i = 0; i < h; ++i) {
        printf("  ");
    }
    printf("%s", v->name);
    if (v->is_dir) {
        printf("/\n");

        DIR* dir = opendir_inode(v);

        struct inode* ch = readdir(dir);
        while (ch != NULL) {
            print_tree(ch, h + 1);
            ch = readdir(dir);
        }
        closedir(dir);
    } else {
        printf("\n");
    }
}

void print_file_system() {
    print_tree(root, 0);
}

int open(const char* pathname, int flags) {
    start_critical_section();
    int id = find_first_free();

    if (((flags & O_RDONLY) != 0) + ((flags & O_WRONLY) != 0) + ((flags & O_RDWR) != 0) != 1) {
        return -1;
    }

    int force = 0;
    if (flags & O_CREAT) {
        ++force;
        if (flags & O_EXCL) {
            ++force;
        }
    }
    file_descriptors[id].node = find_file(pathname, force, create_new_file);

    if (file_descriptors[id].node == NULL) {
        return -1;
    }
    file_descriptors[id].flags = flags;
    file_descriptors[id].current_position = 0;
    if (flags & O_TRUNC) {
        file_descriptors[id].node->size = 0;
    }

    if (flags & O_APPEND) {
        file_descriptors[id].current_position = file_descriptors[id].node->size;
    }
    file_descriptors[id].is_free = 0;

    end_critical_section();
    return id;
}

int correct_fildes(int id) {
    if (id < 0 || id >= MAX_FILE_DIS_COUNT) {
        return 0;
    }

    if (file_descriptors[id].is_free) {
        return 0;
    }
    return 1;
}

int close(int fd) {
    start_critical_section();
    if (!correct_fildes(fd)) {
        end_critical_section();
        return -1;
    }

    file_descriptors[fd].is_free = 1;
    end_critical_section();
    return 0;
}

ssize_t read(int fildes, void* buf, size_t nbyte) {
    start_critical_section();
    if (!correct_fildes(fildes) || (file_descriptors[fildes].flags & O_WRONLY)) {
        end_critical_section();
        return -1;
    }

    struct inode* node = file_descriptors[fildes].node;

    size_t i = 0;

    for (; i < nbyte && file_descriptors[fildes].current_position < node->size; ++i) {
        ((char*)buf)[i] = *(((char*)node->file_start) + file_descriptors[fildes].current_position);
        file_descriptors[fildes].current_position++;
    }
    end_critical_section();
    return i;
}

void reallocate(struct inode *node) {
    void* new_page = get_page0(node->capacity_level + 1);

    node->capacity_level++;
    for (size_t i = 0; i < node->size; ++i) {
        ((char*)new_page)[i] = ((char*)node->file_start)[i];
    }

    free_page(node->file_start, node->capacity_level - 1);
    node->file_start = new_page;
}

ssize_t write(int fildes, const void* buf, size_t nbyte) {
    start_critical_section();
    if (!correct_fildes(fildes) || (file_descriptors[fildes].flags & O_RDONLY)) {
        end_critical_section();
        return -1;
    }

    struct inode* node = file_descriptors[fildes].node;
    size_t i = 0;

    for (; i < nbyte; ++i) {
        if (file_descriptors[fildes].current_position == node->size) {
            ++node->size;
        }
        if (file_descriptors[fildes].current_position == (1LLU << node->capacity_level)*PAGE_SIZE) {
            reallocate(node);
        }

        *(((char*)node->file_start) + file_descriptors[fildes].current_position) = ((char*)buf)[i];
        file_descriptors[fildes].current_position++;
    }

    end_critical_section();
    return nbyte;
}

int mkdir(const char *path) {
    struct inode* node = find_file(path, 2, create_new_dir);
    if (node == NULL) {
        return -1;
    } else {
        return 0;
    }
}

DIR* opendir(const char * path) {
    start_critical_section();
    struct inode* node = find_file(path, 0, NULL);
    DIR* res = opendir_inode(node);
    end_critical_section();
    return res;
}

struct inode* readdir(DIR* dirp) {
    start_critical_section();
    struct inode* res = *dirp;
    if (res == NULL) {
        end_critical_section();
        return NULL;
    }
    *dirp = res->neighbor;
    end_critical_section();
    return res;
}

void closedir (DIR* dir) {
    free_block(dir);
}