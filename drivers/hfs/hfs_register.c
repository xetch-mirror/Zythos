#include "hfs_register.h"
#include "memory.h"

/* Symbols provided by the linker script marking the start/end of
 * the hfs_nodes section. */
extern hfs_static_entry_t __start_hfs_nodes[];
extern hfs_static_entry_t __stop_hfs_nodes[];

void hfs_register_apply(hfs_node_t *root)
{
    for (hfs_static_entry_t *e = __start_hfs_nodes; e < __stop_hfs_nodes; e++) {
        hfs_node_t *parent = root;

        if (e->parent_path && e->parent_path[0] != '\0') {
            hfs_node_t *found = hfs_find_path(root, e->parent_path);
            if (found) {
                parent = found;
            }
            /* else: parent not found yet — falls back to attaching
             * under root rather than silently dropping the node.
             * Fine for a flat registry; matters more once
             * hfs_find_path actually parses multi-segment paths. */
        }

        hfs_node_t *node = hfs_create_node(e->name, e->data);
        if (node) {
            hfs_add_child(parent, node);
        }
    }
}