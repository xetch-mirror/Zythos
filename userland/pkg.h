#ifndef PKG_H
#define PKG_H

#include "vftypes.h"

#define PKG_NAME_MAX 32
#define PKG_PATH_MAX 64
#define PKG_MAX_ENTRIES 64
#define PKG_BIN_DIR "/bin"
#define PKG_DB_PATH "/pkg/installed.db"

typedef struct {
    char name[PKG_NAME_MAX];
    char path[PKG_PATH_MAX];
    uint32_t size;
} pkg_entry_t;

/* copies src_path into /bin/<name>, appends a record to the pkg db,
   and registers <name> as a runnable command (see register.c) */
int pkg_install(const char *src_path, const char *name);

/* removes /bin/<name>, drops its db record, unregisters the command */
int pkg_remove(const char *name);

/* fills out[] with up to max installed entries, returns count */
int pkg_list(pkg_entry_t *out, int max);

#endif