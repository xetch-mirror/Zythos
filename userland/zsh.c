/* zsh.c - Minimal userland shell on nolibc */

#include <nolibc.h>
#include <linux/signal.h>
#include <linux/serial.h>

#define MAX_INPUT   256
#define MAX_ARGS    16
#define MAX_ENV     32
#define ENV_POOL_SZ 2048

#define STDIN_FILENO  0
#define STDOUT_FILENO 1

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
    while (*s) {
        if (*s == c) return s;
        s++;
    }
    return NULL;
}

static int parse_args(char *line, char **args)
{
    int count = 0;
    char *p = line;

    while (*p && count < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
            *p = '\0';
            p++;
        }
        if (*p == '\0') break;

        args[count++] = p;

        while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
            p++;
        }
    }
    args[count] = NULL;
    return count;
}

/* ---- Хранилище export-переменных: без malloc, фиксированный пул ---- */

static char g_env_pool[ENV_POOL_SZ];
static size_t g_env_pool_used = 0;
static char *g_env_slots[MAX_ENV];
static int g_env_count = 0;

/* Копирует "VAR=VALUE" в статический пул и добавляет в g_env_slots.
   Возвращает 0 при успехе, -1 если место закончилось (пул или слоты). */
static int env_set(const char *entry)
{
    size_t len = 0;
    while (entry[len]) len++;

    if (g_env_count >= MAX_ENV - 1) {
        return -1; /* нет свободных слотов */
    }
    if (g_env_pool_used + len + 1 > ENV_POOL_SZ) {
        return -1; /* пул исчерпан */
    }

    char *dst = &g_env_pool[g_env_pool_used];
    for (size_t i = 0; i <= len; i++) {
        dst[i] = entry[i];
    }
    g_env_pool_used += len + 1;

    g_env_slots[g_env_count++] = dst;
    g_env_slots[g_env_count] = NULL;

    return 0;
}

static void env_print_all(void)
{
    for (int i = 0; i < g_env_count; i++) {
        print(g_env_slots[i]);
        print("\n");
    }
}

int main(int argc, char **argv, char **envp)
{
    (void)argc;
    (void)argv;
    (void)envp; /* TODO: изначальный envp от загрузчика пока не копируется
                   в g_env_slots при старте — export работает только с тем,
                   что установлено уже во время работы шелла */

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

        if (streq(args[0], "clear")) {
            print("\033[H\033[2J");
        }
        else if (streq(args[0], "echo")) {
            for (int i = 1; i < arg_count; i++) {
                print(args[i]);
                if (i < arg_count - 1) print(" ");
            }
            print("\n");
        }
        else if (streq(args[0], "export")) {
            if (arg_count < 2) {
                print("export: missing VAR=VALUE\n");
            } else {
                char *eq = find_char(args[1], '=');
                if (!eq) {
                    print("export: usage: export VAR=VALUE\n");
                } else if (env_set(args[1]) == 0) {
                    print("[export set]\n");
                } else {
                    print("export: environment full\n");
                }
            }
        }
        else if (streq(args[0], "env")) {
            env_print_all();
        }
        else if (streq(args[0], "exit")) {
            break;
        }
        else {
            print("shell: command not found: ");
            print(args[0]);
            print("\n");
        }
    }

    return 0;
}