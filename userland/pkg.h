// pkg.h
typedef struct { char name[32]; char path[64]; uint32_t size; } pkg_entry_t;
int pkg_install(const char *src_path, const char *name);
int pkg_remove(const char *name);
int pkg_list(pkg_entry_t *out, int max);