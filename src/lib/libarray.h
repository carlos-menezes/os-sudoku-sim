#ifndef LIBARRAY_H
#define LIBARRAY_H

typedef struct Node
{
    void *value;
    struct Node *next;
} node_t;

typedef struct LinkedList
{
    int size;
    node_t *head;
} linked_list_t;

int ll_init(linked_list_t **l);
// int ll_insert_first(linked_list_t **l, void* value);
int ll_insert(linked_list_t **l, void *value);
int ll_delete_value(linked_list_t **l, void *value);
int ll_free(linked_list_t **l);

#endif