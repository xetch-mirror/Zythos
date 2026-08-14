/* zsh.c - Minimal userland shell on nolibc */

#include <nolibc.h>
#include <linux/signal.h>
#include <linux/serial.h>
#include "cmdtable.h"
#include "register.h"
#include "pkg.h"

#define MAX_INPUT   256
#define MAX_ARGS    16
#define MAX_ENV     32
#define ENV_POOL_SZ 2048

#define STDIN_FILENO  0
#define STDOUT_FILENO 1

#define __NR_exec 3

static void print(const char *str)
{
    size_t len = 0;
    while (str[len]) len++;
    write(STDOUT_FILENO, str, len);
}

static int streq(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static char *find_char(char *s, char c)
{
    while (*s) { if (*s == c) return s; s++; }
    return NULL;
}

static int starts_with(const char *s, const char *prefix)
{
    while (*prefix) { if (*s != *prefix) return 0; s++; prefix++; }
    return 1;
}

/* quote-aware tokenizer: "a b" stays one token */
static int parse_args(char *line, char **args)
{
    int count = 0;
    char *p = line;

    while (*p && count < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (!*p) break;

        if (*p == '"') {
            p++;
            args[count++] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') { *p = '\0'; p++; }
        } else {
            args[count++] = p;
            while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') p++;
            if (*p) { *p = '\0'; p++; }
        }
    }
    args[count] = NULL;
    return count;
}

static long sys_exec_call(const char *path)
{
    long ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(__NR_exec), "b"(path)
        : "memory"
    );
    return ret;
}

/* ---- env storage (unchanged) ---- */
static char g_env_pool[ENV_POOL_SZ];
static size_t g_env_pool_used = 0;
static char *g_env_slots[MAX_ENV];
static int g_env_count = 0;

static int env_set(const char *entry)
{
    size_t len = 0;
    while (entry[len]) len++;
    if (g_env_count >= MAX_ENV - 1) return -1;
    if (g_env_pool_used + len + 1 > ENV_POOL_SZ) return -1;

    char *dst = &g_env_pool[g_env_pool_used];
    for (size_t i = 0; i <= len; i++) dst[i] = entry[i];
    g_env_pool_used += len + 1;

    g_env_slots[g_env_count++] = dst;
    g_env_slots[g_env_count] = NULL;
    return 0;
}

static void env_print_all(void)
{
    for (int i = 0; i < g_env_count; i++) { print(g_env_slots[i]); print("\n"); }
}

/* ---- registered commands ---- */

static int cmd_clear(int argc, char **argv)
{ (void)argc; (void)argv; print("\033[H\033[2J"); return 0; }
REGISTER_CMD("clear", cmd_clear)

static int cmd_echo(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        print(argv[i]);
        if (i < argc - 1) print(" ");
    }
    print("\n");
    return 0;
}
REGISTER_CMD("echo", cmd_echo)

static int cmd_export(int argc, char **argv)
{
    if (argc < 2) { print("export: missing VAR=VALUE\n"); return -1; }
    char *eq = find_char(argv[1], '=');
    if (!eq) { print("export: usage: export VAR=VALUE\n"); return -1; }
    if (env_set(argv[1]) != 0) { print("export: environment full\n"); return -1; }
    print("[export set]\n");
    return 0;
}
REGISTER_CMD("export", cmd_export)

static int cmd_env(int argc, char **argv)
{ (void)argc; (void)argv; env_print_all(); return 0; }
REGISTER_CMD("env", cmd_env)

static int cmd_elf(int argc, char **argv)
{
    if (argc < 2) { print("elf: usage: elf <path>\n"); return -1; }
    long ret = sys_exec_call(argv[1]);
    if (ret != 0) { print("elf: failed to load "); print(argv[1]); print("\n"); }
    return (int)ret;
}
REGISTER_CMD("elf", cmd_elf)

static int cmd_pkg(int argc, char **argv)
{
    if (argc < 2) { print("usage: pkg install <src> <name> | remove <name> | list\n"); return -1; }
    if (streq(argv[1], "install") == 0 && argc >= 4) return pkg_install(argv[2], argv[3]);
    if (streq(argv[1], "remove") == 0 && argc >= 3) return pkg_remove(argv[2]);
    if (streq(argv[1], "list") == 0) {
        pkg_entry_t entries[PKG_MAX_ENTRIES];
        int n = pkg_list(entries, PKG_MAX_ENTRIES);
        for (int i = 0; i < n; i++) {
            print(entries[i].name); print("  "); print(entries[i].path); print("\n");
        }
        return 0;
    }
    print("pkg: unknown subcommand\n");
    return -1;
}
REGISTER_CMD("pkg", cmd_pkg)

/* exit is handled specially in the loop (needs to break out of main),
   so it stays outside the cmdtable rather than returning through fn() */

static void dispatch(char *args0, int arg_count, char **args)
{
    for (cmd_entry_t *c = __start_cmdtable; c < __stop_cmdtable; c++) {
        if (streq(c->name, args0) == 0) {
            c->fn(arg_count, args);
            return;
        }
    }

    char pkg_path[64];
    if (dyn_lookup(args0, pkg_path, sizeof(pkg_path))) {
        long ret = sys_exec_call(pkg_path);
        if (ret != 0) { print(args0); print(": exec failed\n"); }
        return;
    }

    if (starts_with(args0, "./") || args0[0] == '/') {
        long ret = sys_exec_call(args0);
        if (ret != 0) { print(args0); print(": exec failed\n"); }
        return;
    }

    print("zsh: command not found: ");
    print(args0);
    print("\n");
}

int main(int argc, char **argv, char **envp)
{
    (void)argc; (void)argv; (void)envp;

    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    serial_init();
    serial_puts("[INIT] Userland shell initialized.\n");

    while (1) {
        print("$ ");

        long bytes = read(STDIN_FILENO, input, sizeof(input) - 1);
        if (bytes <= 0) break;

        input[bytes] = '\0';

        int arg_count = parse_args(input, args);
        if (arg_count == 0) continue;

        if (streq(args[0], "exit") == 0) break;

        dispatch(args[0], arg_count, args);
    }

    return 0;
}