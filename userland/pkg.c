/* Локальный менеджер пакетов: копирует бинарник в /bin и запоминает
   его в плоской базе данных. Нет сети, нет версий, нет зависимостей —
   просто "установлен / не установлен" и путь. */

#include "pkg.h"
#include "vfs.h"
#include "sys_io.h"
#include "syscalls.h"
#include "memory.h"
#include "register.h"

static int str_eq(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int str_len(const char *s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void str_cpy(char *dst, const char *src, int max)
{
    int i = 0;
    while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

/* читает всю db в память; возвращает количество записей */
static int db_load(pkg_entry_t *out, int max)
{
    if (!vfs_exists(PKG_DB_PATH)) return 0;

    vfs_file_t f;
    if (vfs_open(PKG_DB_PATH, &f) != 0) return 0;

    int count = 0;
    while (count < max) {
        int n = vfs_read(&f, &out[count], sizeof(pkg_entry_t));
        if (n != (int)sizeof(pkg_entry_t)) break;
        count++;
    }
    return count;
}

static int db_save(pkg_entry_t *entries, int count)
{
    vfs_file_t f;
    if (vfs_open(PKG_DB_PATH, &f) != 0) {
        /* пытаемся создать, если не существует — зависит от того,
           что vfs_open с флагом создания поддерживается сверху;
           TODO: заменить на явный vfs_create, когда появится */
        return -1;
    }
    for (int i = 0; i < count; i++) {
        if (vfs_write(&f, &entries[i], sizeof(pkg_entry_t)) != (int)sizeof(pkg_entry_t))
            return -1;
    }
    return 0;
}

int pkg_install(const char *src_path, const char *name)
{
    if (str_len(name) >= PKG_NAME_MAX) return -1;

    vfs_file_t src;
    if (vfs_open(src_path, &src) != 0) return -1;

    char dst_path[PKG_PATH_MAX];
    str_cpy(dst_path, PKG_BIN_DIR, PKG_PATH_MAX);
    int len = str_len(dst_path);
    dst_path[len] = '/';
    str_cpy(dst_path + len + 1, name, PKG_PATH_MAX - len - 1);

    vfs_file_t dst;
    if (vfs_open(dst_path, &dst) != 0) return -1; /* TODO: needs create-on-open */

    uint8_t buf[512];
    uint32_t total = 0;
    int n;
    while ((n = vfs_read(&src, buf, sizeof(buf))) > 0) {
        if (vfs_write(&dst, buf, n) != n) return -1;
        total += n;
    }

    pkg_entry_t entries[PKG_MAX_ENTRIES];
    int count = db_load(entries, PKG_MAX_ENTRIES);

    for (int i = 0; i < count; i++) {
        if (str_eq(entries[i].name, name)) {
            entries[i].size = total;
            str_cpy(entries[i].path, dst_path, PKG_PATH_MAX);
            db_save(entries, count);
            register_elf_command(name, dst_path);
            return 0;
        }
    }

    if (count >= PKG_MAX_ENTRIES) return -1;

    str_cpy(entries[count].name, name, PKG_NAME_MAX);
    str_cpy(entries[count].path, dst_path, PKG_PATH_MAX);
    entries[count].size = total;
    count++;

    if (db_save(entries, count) != 0) return -1;

    register_elf_command(name, dst_path);
    return 0;
}

int pkg_remove(const char *name)
{
    pkg_entry_t entries[PKG_MAX_ENTRIES];
    int count = db_load(entries, PKG_MAX_ENTRIES);

    int found = -1;
    for (int i = 0; i < count; i++) {
        if (str_eq(entries[i].name, name)) { found = i; break; }
    }
    if (found < 0) return -1;

    /* TODO: no vfs_unlink() implementation yet to actually delete
       the /bin/<name> file — db record is dropped, file stays orphaned */

    for (int i = found; i < count - 1; i++) entries[i] = entries[i + 1];
    count--;

    unregister_command(name);
    return db_save(entries, count);
}

int pkg_list(pkg_entry_t *out, int max)
{
    return db_load(out, max);
}