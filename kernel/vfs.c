#include "vfs.h"

static vfs_node_t vfs_nodes[VFS_MAX_NODES];
static uint32_t vfs_node_count;

void vfs_init(void)
{
    vfs_node_count = 0;
}

int vfs_register_node(vfs_node_t node)
{
    if (vfs_node_count >= VFS_MAX_NODES) {
        return VFS_ENOMEM;
    }

    vfs_nodes[vfs_node_count++] = node;
    return VFS_OK;
}

vfs_node_t *vfs_lookup(const char *path)
{
    if (!path) {
        return 0;
    }

    for (uint32_t index = 0; index < vfs_node_count; index++) {
        if (vfs_strcmp(vfs_nodes[index].name, path) == 0) {
            return &vfs_nodes[index];
        }
    }

    return 0;
}

int vfs_exists(const char *path)
{
    return vfs_lookup(path) != 0;
}

int vfs_mkdir(const char *path)
{
    vfs_node_t node = {0};

    if (!path || vfs_exists(path)) {
        return VFS_ERROR;
    }

    node.flags = VFS_DIRECTORY;
    node.ops = 0;
    for (uint32_t index = 0; path[index] && index < VFS_MAX_PATH - 1; index++) {
        node.name[index] = path[index];
    }

    return vfs_register_node(node);
}

void vfs_list_dir(const char *path)
{
    (void)path;
}
