/* Мост между статической cmdtable (REGISTER_CMD, секция линкера)
   и динамически устанавливаемыми pkg-командами. pkg-команды не
   попадают в секцию .cmdtable (та собирается на этапе линковки) —
   вместо этого они живут в отдельной таблице в памяти времени
   выполнения. */

#include "register.h"
#include "cmdtable.h"

typedef struct {
    char name[32];
    char path[64];
    int used;
} dyn_entry_t;

static dyn_entry_t g_dyn_table[DYN_CMD_MAX];

extern cmd_entry_t __start_cmdtable[];
extern cmd_entry_t __stop_cmdtable[];

static int str_eq(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static void str_cpy(char *dst, const char *src, int max)
{
    int i = 0;
    while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

int register_elf_command(const char *name, const char *path)
{
    for (int i = 0; i < DYN_CMD_MAX; i++) {
        if (g_dyn_table[i].used && str_eq(g_dyn_table[i].name, name)) {
            str_cpy(g_dyn_table[i].path, path, 64);
            return 0;
        }
    }
    for (int i = 0; i < DYN_CMD_MAX; i++) {
        if (!g_dyn_table[i].used) {
            g_dyn_table[i].used = 1;
            str_cpy(g_dyn_table[i].name, name, 32);
            str_cpy(g_dyn_table[i].path, path, 64);
            return 0;
        }
    }
    return -1; /* table full */
}

int unregister_command(const char *name)
{
    for (int i = 0; i < DYN_CMD_MAX; i++) {
        if (g_dyn_table[i].used && str_eq(g_dyn_table[i].name, name)) {
            g_dyn_table[i].used = 0;
            return 0;
        }
    }
    return -1;
}

int cmd_lookup(const char *name, cmd_fn_t *fn, char *path_out, int path_max, int *is_elf)
{
    for (cmd_entry_t *c = __start_cmdtable; c < __stop_cmdtable; c++) {
        if (str_eq(c->name, name)) {
            *fn = c->fn;
            *is_elf = 0;
            return 1;
        }
    }
    for (int i = 0; i < DYN_CMD_MAX; i++) {
        if (g_dyn_table[i].used && str_eq(g_dyn_table[i].name, name)) {
            str_cpy(path_out, g_dyn_table[i].path, path_max);
            *is_elf = 1;
            return 1;
        }
    }
    return 0;
}