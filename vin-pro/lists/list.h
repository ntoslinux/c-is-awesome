#ifndef _LIST_H
#define _LIST_H

struct list_node {
        struct list_node *next;
        struct list_node *prev;
};

void list_init(struct list_node *node);
void list_add_after(struct list_node *new, struct list_node *node);
void list_add_before(struct list_node *new, struct list_node *node);

#endif