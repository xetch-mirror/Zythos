#ifndef REGISTER_H
#define REGISTER_H

/* dynamic counterpart to REGISTER_CMD — for commands that aren't
   known at compile time (pkg-installed binaries), rather than
   living in the static linker-section cmdtable */

typedef int (*cmd_fn_t)(int argc, char **argv);

#define DYN_CMD_MAX 32

/* registers name to exec dst_path via SYS_EXEC when typed in zsh */
int register_elf_command(const char *name, const char *path);
int unregister_command(const char *name);

/* looks up name in BOTH the static cmdtable and the dynamic table;
   returns 1 and fills *is_elf/path if it's a pkg-registered binary,
   or fills *fn if it's a static REGISTER_CMD entry. returns 0 if not found */
int cmd_lookup(const char *name, cmd_fn_t *fn, char *path_out, int path_max, int *is_elf);

#endif