#include "hfs.h"
#include "memory.h"
#include "types.h"

/* Простая проверка строк без libc */
static bool str_eq(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b) return false;
        a++; b++;
    }
    return *a == *b;
}

void hfs_init(hfs_node_t *root, const char *name)
{
    root->name = name;
    root->data = 0;
    root->parent = 0;
    root->first_child = 0;
    root->next_sibling = 0;
}

hfs_node_t *hfs_create_node(const char *name, void *data)
{
    hfs_node_t *node = (hfs_node_t *)kmalloc(sizeof(hfs_node_t));
    if (!node) {
        return 0;
    }

    node->name = name;
    node->data = data;
    node->parent = 0;
    node->first_child = 0;
    node->next_sibling = 0;

    return node;
}

bool hfs_add_child(hfs_node_t *parent, hfs_node_t *child)
{
    if (!parent || !child) {
        return false;
    }

    child->parent = parent;
    child->next_sibling = parent->first_child;
    parent->first_child = child;

    return true;
}

hfs_node_t *hfs_find_child(hfs_node_t *parent, const char *name)
{
    hfs_node_t *current = parent->first_child;

    while (current != 0) {
        if (str_eq(current->name, name)) {
            return current;
        }
        current = current->next_sibling;
    }

    return 0;
}

hfs_node_t *hfs_find_path(hfs_node_t *root, const char *path)
{
    /* TODO: разбор пути "a/b/c" по сегментам — пока не реализовано,
       ищет только прямого потомка по одному имени */
    return hfs_find_child(root, path);
}

void hfs_remove(hfs_node_t *node)
{
    if (!node || !node->parent) {
        return;
    }

    hfs_node_t *parent = node->parent;
    hfs_node_t *current = parent->first_child;

    if (current == node) {
        parent->first_child = node->next_sibling;
    } else {
        while (current != 0 && current->next_sibling != node) {
            current = current->next_sibling;
        }
        if (current != 0) {
            current->next_sibling = node->next_sibling;
        }
    }

    kfree(node);
}