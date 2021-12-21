#ifndef LIBARRAY_H
#define LIBARRAY_H

struct node_t
{
    void *value;
    struct node_t *next;
};

struct linked_list_t
{
    int size;
    struct node_t *head;
};

int ll_init(struct linked_list_t **l);
int ll_insert(struct linked_list_t **l, void *value);
int ll_delete_value(struct linked_list_t **l, void *value);
int ll_free(struct linked_list_t **l);

#endif