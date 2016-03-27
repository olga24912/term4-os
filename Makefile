CC ?= gcc
LD ?= gcc

CFLAGS := -g -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding \
	-mcmodel=kernel -Wall -Wextra -Werror -std=c99 \
	-Wframe-larger-than=4096 -Wstack-usage=4096 -Wno-unknown-warning-option
LFLAGS := -nostdlib -z max-page-size=0x1000

ASM := bootstrap.S videomem.S isr_wrapper.S threads_util.S
AOBJ:= $(ASM:%.S=.build/%.o)
ADEP:= $(ASM:%.S=.build/%.d)

SRC := main.c uart.c io.c interrupt.c timer.c memory_map.c buddy_allocator.c paging.c SLAB_allocator.c threads.c lock.c
COBJ := $(SRC:%.c=.build/%.o)
CDEP := $(SRC:%.c=.build/%.d)

OBJ := $(AOBJ) $(COBJ)
DEP := $(ADEP) $(CDEP)

all: kernel

isr_wrapper.S: gen_isr_wrapper.py
	./$^ >$@

make_idt.h: gen_make_idt.py
	./$^ >$@

interrupt.o: make_idt.h

kernel: $(OBJ) kernel.ld
	$(LD) $(LFLAGS) -T kernel.ld -Map kernel.map -o $@ $(OBJ)

$(AOBJ): .build/%.o : %.S | .build
	$(CC) -D__ASM_FILE__ -g -MMD -c $< -o $@

$(COBJ): .build/%.o : %.c | .build
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(DEP)

.build:
	mkdir .build

.PHONY: clean
clean:
	rm -rf kernel .build kernel.map
