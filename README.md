<p align="center">
  <img src="Your paragraph text.png" width="220" alt="Zythos logo">
</p>

<h1 align="center">Zythos</h1>
<p align="center"><b>A minimal, embeddable operating system written from scratch in C.</b></p>

<p align="center">
  <img src="https://img.shields.io/badge/license-GPL--2.0-blue" alt="License: GPL-2.0">
  <img src="https://img.shields.io/badge/arch-x86%20(i686)-informational" alt="Architecture: x86">
  <img src="https://img.shields.io/badge/status-1.0%20released-brightgreen">
</p>

---

## About

Zythos is a from-scratch, x86 (32-bit protected mode) operating system built for
minimalism and embeddability. It boots via a custom real-mode stage1/stage2
bootloader, runs a hand-written kernel, and is paired with its own toolchain —
including a compiler, assembler, linker, C library, and coreutils — built
specifically for the system rather than ported wholesale from Linux.

powerpc / powerpc64 support is planned as a future architecture target.

## Features

- **Custom bootloader** — 16-bit real-mode stage1 (FAT12/16/32 BPB, BIOS disk
  read via CHS/LBA) and stage2, loading the kernel at `0x100000`
- **Kernel** — memory management (custom `kmalloc`/`kfree` allocator), IDT and
  `int 0x80` syscall dispatch, ELF32 loader
- **BCC** — a minimal, purpose-built C compiler/interpreter (inspired by `c4`)
- **Own toolchain** — an in-progress `binutils` suite: a freestanding
  assembler with a custom object format, and a linker based on `mold`
- **libc** — a custom C library with multi-arch raw syscall wrappers
- **Userland** — a non-Unix shell (`BlShell`), a minimal `nolibc`-based shell,
  and coreutils (including `ls`, built directly against FAT32 structures)
- **ttar** — a "Tiny tar" implementation supporting create/extract of
  `ustar`-format archives
- **sysfetch** — a `neofetch`-style system info tool
- Embedded Lua scripting support (in progress)

## Repository Layout

```
arch/      # architecture-specific code (boot.asm, etc.)
base/      # base subsystem tree
drivers/   # device drivers
fs/        # filesystem code (FAT32; ext2 under consideration)
include/   # shared headers (kernel + imported Linux-style headers)
init/      # kernel init sequence
input/     # input handling (keyboard, etc.)
kernel/    # kernel core (main.c, mm, syscalls, IDT)
```

## Building

> Zythos is developed in a Linux container (GitHub Codespaces). Build
> instructions and toolchain requirements are being finalized as the custom
> `binutils` suite matures.

```sh
# placeholder — update once the build script is finalized
make
```

This produces a bootable disk image that can be run in an emulator such as
QEMU:

```sh
qemu-system-i386 -drive format=raw,file=zythos.img
```

## Status

Zythos currently boots and runs a working userland shell with basic syscalls
(`read`/`write`/`exit`), memory allocation, and an ELF32 loader. Development is
active and many subsystems (filesystem choice, paging/VMM, process model) are
still evolving.

## Credits

- `keyboard.h`, `keycodes.h`, and `keymap.h` originally from
  [BoredOS](https://github.com/lluciocc) by Lluciocc, used under GPL.
- Bootloader design informed by the NanoByte OS tutorial series.

## License

GPL-2.0
