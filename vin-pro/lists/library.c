#include <stddef.h>
#include <stdio.h>
#include "list.h"

// The below code demonstrates using doubly linked list as a struct containing
// list of other objects. Here struct library contains list of books. It does
// not demonstrate using a self contains list of books without using the special
// head.

struct library {
        // int some_other_fields;

        // Here we are using book_list as the head of the books list. It is a
        // dummy special node not used in actual book object inspection. So
        // everytime we iterate the books list we start from book_list's next
        // pointer and loop until the current node is not book_list. THIS IS
        // VERY IMPORTANT
        struct list_node book_list;
};

struct book {
        int price;
        char name[100];

        // Below is the actual list node which will make up the book nodes
        struct list_node node;
};

int embedded_list_of_objects()
{
        struct library lib = {0};
        struct book b1 = {100, "c programming", 0};
        struct book b2 = {200, "java programming", 0};
        struct book b3 = {300, "C# programming", 0};
        struct book b4 = {400, "lua programming", 0};

        list_init(&lib.book_list);
        list_init(&b1.node);
        list_init(&b2.node);
        list_init(&b3.node);
        list_init(&b4.node);

        // This will create a stack of books inserted as below

        // .-> book_list <-> b4 <-> b3 <-> b2 <-> b1 <-.
        // '-------------------------------------------'

        // list_add_after(&b1.node, &lib.book_list);
        // list_add_after(&b2.node, &lib.book_list);
        // list_add_after(&b3.node, &lib.book_list);
        // list_add_after(&b4.node, &lib.book_list);

        // This will create a queue of books inserted as below
        // .->b1 <-> b2 <-> b3 <-> b4 <-> book_list <-.
        // '------------------------------------------'

        list_add_before(&b1.node, &lib.book_list);
        list_add_before(&b2.node, &lib.book_list);
        list_add_before(&b3.node, &lib.book_list);
        list_add_before(&b4.node, &lib.book_list);


        // Below intialization and condition are very important
        for (struct list_node *list = lib.book_list.next;
                list != &lib.book_list;
                list = list->next) {

                struct book *b = (struct book*)((char*)list - offsetof(struct book, node));
                printf("\nname = %s, price = %d", b->name, b->price);
        }

        return 0;
}