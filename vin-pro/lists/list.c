#include <stddef.h>

#include "list.h"

void list_init(struct list_node *node)
{
        node->next = node->prev = node;
}

static void __list_add(struct list_node *new, struct list_node *n1, struct list_node *n2)
{
        new->next = n2;
        new->prev = n1;
        n1->next = new;
        n2->prev = new;
}

void list_add_after(struct list_node *new, struct list_node *node)
{
        __list_add(new, node, node->next);
}

void list_add_before(struct list_node *new, struct list_node *node)
{
        __list_add(new, node->prev, node);
}

static void __list_del(struct list_node *node, struct list_node *n1, struct list_node *n2)
{
        n1->next = n2;
        n2->prev = n1;

        node->next = NULL;
        node->prev = NULL;
}

void list_del(struct list_node *node)
{
        __list_del(node, node->prev, node->next);
}
