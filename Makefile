.DEFAULT_GOAL := all

BUILD_DIR := build
BOOT_DIR := $(BUILD_DIR)/boot
BOOT_IMAGE := $(BOOT_DIR)/zythos.img
BOOT_STAGE1_OBJ := $(BOOT_DIR)/stage1.o
BOOT_STAGE1 := $(BOOT_DIR)/stage1.bin
BOOT_STAGE2 := $(BOOT_DIR)/stage2.bin
BOOT_STAGE2_SECTORS := 15
KERNEL_DIR := $(BUILD_DIR)/kernel
KERNEL_ELF := $(KERNEL_DIR)/kernel.elf
KERNEL_BIN := $(KERNEL_DIR)/kernel.bin
KERNEL_SECTORS := 16
KERNEL_LBA := 16
FS_DIR := $(BUILD_DIR)/fs
FS_IMAGE := $(FS_DIR)/zythos.img
FS_STAGE := $(FS_DIR)/root
OBJECT_DIR := $(BUILD_DIR)/objects
ARTIFACT_DIR := $(BUILD_DIR)/artifacts
MM_ELF := $(ARTIFACT_DIR)/mm.elf
INIT_ELF := $(ARTIFACT_DIR)/init.elf
INPUT_ELF := $(ARTIFACT_DIR)/input.elf
INITRD := $(ARTIFACT_DIR)/initrd.tar.gz
INITLIB_HEADER := $(BUILD_DIR)/include/initlib.h
COREUTILS_DIR := usr/usr/coreutils
COREUTILS_BUILD := $(COREUTILS_DIR)/build
COREUTILS_SOURCES := $(COREUTILS_DIR)/src/cat.c $(COREUTILS_DIR)/src/grep.c $(COREUTILS_DIR)/src/ls.c
COREUTILS_OBJECTS := $(patsubst $(COREUTILS_DIR)/src/%.c,$(COREUTILS_BUILD)/src/%.o,$(COREUTILS_SOURCES))
COREUTILS_LIB := $(COREUTILS_BUILD)/libcoreutils.a

KERNEL_SOURCES := kernel/main.c kernel/vfs.c drivers/loader.c
MM_SOURCES := mm/blocks.c mm/heap.c
INIT_SOURCES := init/init.c init/init_proc.c init/initlib.c
INPUT_SOURCES := input/keyboard.c input/keymap.c
COMPILE_SOURCES := $(KERNEL_SOURCES) $(MM_SOURCES) $(INIT_SOURCES) $(INPUT_SOURCES)
COMPILE_OBJECTS := $(patsubst %.c,$(OBJECT_DIR)/%.o,$(COMPILE_SOURCES))
HEADER_SOURCES := $(shell find include -type f -name '*.h' -print)

.PHONY: all help headers compile submodules userspace bcc zlibc coreutils boot fs run clean check-tools check-fs-tools

all: headers compile userspace boot fs

help:
	@printf '%s\n' \
		'Zythos build targets:' \
		'  make headers    Check every project header with the freestanding compiler' \
		'  make compile    Compile all buildable kernel-side C sources' \
		'  make userspace  Build the available user-space components' \
		'  make bcc         Build the bundled C4 compiler' \
		'  make zlibc       Prepare the bundled libc headers' \
		'  make boot        Build the x86 boot image (requires nasm)' \
		'  make fs          Build the FAT32 filesystem image' \
		'  make run         Boot the i386 image with serial output on stdio' \
		'  make clean       Remove generated build output'

headers:
	@set -e; for header in $(HEADER_SOURCES); do \
		printf 'checking %s\n' "$$header"; \
		gcc -m32 -ffreestanding -fsyntax-only -x c -Iinclude -Ikernel/include -Iinit/include "$$header"; \
	done

compile: $(COMPILE_OBJECTS) $(KERNEL_ELF) $(MM_ELF) $(INIT_ELF) $(INPUT_ELF) $(INITRD)

submodules:
	git submodule update --init --recursive

userspace: submodules bcc zlibc $(COREUTILS_LIB)

bcc:
	$(MAKE) -C usr/usr/BCC

zlibc:
	$(MAKE) -C usr/usr/zlibc

coreutils: $(COREUTILS_LIB)

$(COREUTILS_LIB): $(COREUTILS_OBJECTS) | $(COREUTILS_BUILD)
	ar rcs $@ $^

$(COREUTILS_BUILD)/src/%.o: $(COREUTILS_DIR)/src/%.c
	mkdir -p $(dir $@)
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -I$(COREUTILS_DIR)/include -I$(COREUTILS_DIR)/lib -Iusr/usr/zlibc/sysroot/include -c $< -o $@

fs: check-fs-tools userspace compile $(FS_IMAGE)

$(FS_IMAGE): userspace $(KERNEL_ELF) $(MM_ELF) $(INIT_ELF) $(INPUT_ELF) $(INITRD) | $(FS_DIR)
	rm -rf $(FS_STAGE)
	mkdir -p $(FS_STAGE)/boot $(FS_STAGE)/usr/bin $(FS_STAGE)/usr/lib $(FS_STAGE)/usr/include
	rsync -a base/ $(FS_STAGE)/
	cp usr/usr/BCC/build/bcc $(FS_STAGE)/usr/bin/bcc
	cp usr/usr/coreutils/build/libcoreutils.a $(FS_STAGE)/usr/lib/libcoreutils.a
	cp -a usr/usr/zlibc/sysroot/include/. $(FS_STAGE)/usr/include/
	cp $(KERNEL_ELF) $(FS_STAGE)/boot/kernel.elf
	cp $(MM_ELF) $(INIT_ELF) $(INPUT_ELF) $(INITRD) $(FS_STAGE)/boot/
	dd if=/dev/zero of=$@ bs=1M count=64 status=none
	mkfs.fat -F 32 -n ZYTHOS $@ >/dev/null
	mcopy -i $@ -s $(FS_STAGE)/* ::/

$(FS_DIR):
	mkdir -p $@

boot: check-tools $(BOOT_IMAGE)

run: headers compile userspace fs $(BOOT_IMAGE) check-tools
	qemu-system-i386 -display none -serial stdio -drive format=raw,file=$(BOOT_IMAGE)

$(BOOT_IMAGE): $(BOOT_STAGE1) $(BOOT_STAGE2) $(KERNEL_BIN) | $(BOOT_DIR)
	dd if=/dev/zero of=$@ bs=512 count=$$(($(KERNEL_LBA) + $(KERNEL_SECTORS))) status=none
	dd if=$(BOOT_STAGE1) of=$@ conv=notrunc status=none
	dd if=$(BOOT_STAGE2) of=$@ bs=512 seek=1 conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=$(KERNEL_LBA) conv=notrunc status=none

$(BOOT_STAGE1): $(BOOT_STAGE1_OBJ)
	ld -m elf_i386 -T arch/x86-64/linker.ld -o $@ $<

$(BOOT_STAGE1_OBJ): arch/x86-64/main.asm | $(BOOT_DIR)
	nasm -f elf32 -dFILESYSTEM=3 -Iarch/x86-64/ $< -o $@

$(BOOT_STAGE2): arch/x86-64/stage2/stage2.asm arch/x86-64/stage2/disk.asm arch/x86-64/stage2/gdt.asm | $(BOOT_DIR)
	nasm -f bin -dKERNEL_SECTORS=$(KERNEL_SECTORS) -Iarch/x86-64/stage2/ $< -o $@
	@test "$$(wc -c < $@)" -le $$((512 * $(BOOT_STAGE2_SECTORS))) || { printf '%s\n' 'error: stage2 exceeds its reserved disk space' >&2; exit 1; }

$(KERNEL_BIN): $(KERNEL_ELF) | $(KERNEL_DIR)
	objcopy -O binary $(KERNEL_ELF) $@
	@test "$$(wc -c < $@)" -le $$((512 * $(KERNEL_SECTORS))) || { printf '%s\n' 'error: kernel exceeds its reserved disk space' >&2; exit 1; }

$(KERNEL_ELF): $(OBJECT_DIR)/kernel/main.o $(OBJECT_DIR)/kernel/vfs.o $(OBJECT_DIR)/drivers/loader.o $(OBJECT_DIR)/mm/heap.o $(OBJECT_DIR)/mm/blocks.o kernel/linker.ld | $(KERNEL_DIR)
	ld -m elf_i386 -T kernel/linker.ld -o $@ $(OBJECT_DIR)/kernel/main.o $(OBJECT_DIR)/kernel/vfs.o $(OBJECT_DIR)/drivers/loader.o $(OBJECT_DIR)/mm/heap.o $(OBJECT_DIR)/mm/blocks.o

$(OBJECT_DIR)/init/initlib.o: init/initlib.c $(INITLIB_HEADER)
	mkdir -p $(dir $@)
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -I$(BUILD_DIR)/include -Iinclude -Ikernel/include -Iinit/include -c $< -o $@

$(OBJECT_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -Iinclude -Ikernel/include -Iinit/include -c $< -o $@

$(INITLIB_HEADER): init/include/initlib.h | $(BUILD_DIR)/include
	cp "$<" "$@"

$(BUILD_DIR)/include:
	mkdir -p $@

$(MM_ELF): $(patsubst %.c,$(OBJECT_DIR)/%.o,$(MM_SOURCES)) | $(ARTIFACT_DIR)
	ld -m elf_i386 -r -o $@ $^

$(INIT_ELF): $(patsubst %.c,$(OBJECT_DIR)/%.o,$(INIT_SOURCES)) | $(ARTIFACT_DIR)
	ld -m elf_i386 -r -o $@ $^

$(INPUT_ELF): $(patsubst %.c,$(OBJECT_DIR)/%.o,$(INPUT_SOURCES)) | $(ARTIFACT_DIR)
	ld -m elf_i386 -r -o $@ $^

$(INITRD): $(INIT_SOURCES) | $(ARTIFACT_DIR)
	tar -czf $@ -C init .

$(KERNEL_DIR):
	mkdir -p $@

$(ARTIFACT_DIR):
	mkdir -p $@

$(COREUTILS_BUILD):
	mkdir -p $@

$(BOOT_DIR):
	mkdir -p $@

check-tools:
	@command -v nasm >/dev/null || { printf '%s\n' 'error: nasm is required for boot builds' >&2; exit 1; }

check-fs-tools:
	@command -v mkfs.fat >/dev/null || { printf '%s\n' 'error: mkfs.fat is required for filesystem builds (install dosfstools)' >&2; exit 1; }
	@command -v mcopy >/dev/null || { printf '%s\n' 'error: mcopy is required for filesystem builds (install mtools)' >&2; exit 1; }

clean:
	$(MAKE) -C usr/usr/BCC clean
	$(MAKE) -C usr/usr/zlibc clean
	rm -rf $(COREUTILS_BUILD)
	rm -rf $(BUILD_DIR)