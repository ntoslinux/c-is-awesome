#include <stddef.h>
#include <stdio.h>
#include "list.h"

// The below code demonstrates using doubly linked list as a list of same
// objects. It does not use any special head node

struct book {
        int price;
        char name[100];

        // Below is the actual list node which will make up the book nodes
        struct list_node node;
};

int main()
{
        struct book b1 = {100, "c programming", 0};
        struct book b2 = {200, "java programming", 0};
        struct book b3 = {300, "C# programming", 0};
        struct book b4 = {400, "lua programming", 0};

        list_init(&b1.node);
        list_init(&b2.node);
        list_init(&b3.node);
        list_init(&b4.node);

        // This will create a stack of books inserted as below

        // .-> b1 <-> b2 <-> b3 <-> b4 <-.
        // '-----------------------------'

        // list_add_after(&b2.node, &b1.node);
        // list_add_after(&b3.node, &b2.node);
        // list_add_after(&b4.node, &b3.node);

        // This will create a queue of books inserted as below
        // .-> b4 <-> b3 <-> b2 <-> b1 <-.
        // '-----------------------------'

        list_add_before(&b2.node, &b1.node);
        list_add_before(&b3.node, &b2.node);
        list_add_before(&b4.node, &b3.node);


        // b1 is nothing special here. We can pick any node to iterate the
        // entire list
        struct list_node *node = &b1.node;
        do {
                struct book *b = (struct book*)((char*)node - offsetof(struct book, node));
                printf("\nname = %s, price = %d", b->name, b->price);
                node = node->prev;
        } while (node != &b1.node);

        printf("\n");
        return 0;
}