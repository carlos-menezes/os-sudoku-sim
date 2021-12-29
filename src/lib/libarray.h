#ifndef LIBARRAY_H
#define LIBARRAY_H

struct node_t
{
    void *value;
    struct node_t *next;
};

int ll_init(struct node_t **head);
int ll_insert(struct node_t **head, void *value);
int ll_delete_value(struct node_t **head, void *value);;
int ll_size(struct node_t *head);

#endif