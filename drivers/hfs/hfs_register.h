#ifndef HFS_REGISTER_H
#define HFS_REGISTER_H

#include "hfs.h"

/* One static entry describing a node that should exist in the hfs
 * tree at boot. path is a single segment name (child of root, or
 * child of whatever parent_path resolves to). */
typedef struct {
    const char *parent_path;   /* 0 or "" = attach directly under root */
    const char *name;
    void *data;
} hfs_static_entry_t;

/* Each TU that wants a node registered declares one of these.
 * The 'used' attribute stops the linker/compiler from discarding it
 * since nothing else references it directly. */
#define REGISTER_HFS_NODE(id, parent_path_, name_, data_)              \
    static const hfs_static_entry_t _hfs_entry_##id                    \
        __attribute__((section("hfs_nodes"), used)) = {                \
            (parent_path_), (name_), (data_)                           \
        }

/* Walks the hfs_nodes linker section and attaches every registered
 * entry under root (or under parent_path if given). Call once at
 * boot, after hfs_init(root, ...) and before anything queries the
 * tree. */
void hfs_register_apply(hfs_node_t *root);

#endif