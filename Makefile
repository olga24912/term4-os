CC ?= gcc
LD ?= gcc

CFLAGS := -g -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding \
	-mcmodel=kernel -Wall -Wextra -Werror -pedantic -std=c99 \
	-Wframe-larger-than=4096 -Wstack-usage=4096 -Wno-unknown-warning-option
LFLAGS := -nostdlib -z max-page-size=0x1000

ASM := bootstrap.S videomem.S isr_wrapper.S
AOBJ:= $(ASM:.S=.o)
ADEP:= $(ASM:.S=.d)

SRC := main.c uart.c io.c interrupt.c timer.c
OBJ := $(AOBJ) $(SRC:.c=.o)
DEP := $(ADEP) $(SRC:.c=.d)

all: kernel

isr_wrapper.S: gen_isr_wrapper.py
	./$^ >$@

make_idt.h: gen_make_idt.py
	./$^ >$@

interrupt.o: make_idt.h

kernel: $(OBJ) kernel.ld
	$(LD) $(LFLAGS) -T kernel.ld -Map kernel.map -o $@ $(OBJ)

%.o: %.S
	$(CC) -D__ASM_FILE__ -g -MMD -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(DEP)

.PHONY: clean
clean:
	rm -f kernel $(OBJ) $(DEP) kernel.map
