#pragma once
#include "interrupt.h"
#include "io.h"
#include "memory_map.h"
#include "memory.h"

void* get_page(int k);

void free_page(void* page_addr, int k);

void init_buddy();
