// Иерархия объектов ядра для base/ — не файловая система на диске
#ifndef HFS_H
#define HFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct hfs_node {
    const char *name;
    void *data;                 /* произвольные данные, привязанные к узлу */
    struct hfs_node *parent;
    struct hfs_node *first_child;
    struct hfs_node *next_sibling;
} hfs_node_t;

void hfs_init(hfs_node_t *root, const char *name);

hfs_node_t *hfs_create_node(const char *name, void *data);
bool hfs_add_child(hfs_node_t *parent, hfs_node_t *child);
hfs_node_t *hfs_find_child(hfs_node_t *parent, const char *name);
hfs_node_t *hfs_find_path(hfs_node_t *root, const char *path); /* "a/b/c" */

void hfs_remove(hfs_node_t *node);

#endif // HFS_H