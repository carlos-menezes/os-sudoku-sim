#ifndef LIBARRAY_H
#define LIBARRAY_H

#define GUESSES_SIZE 9

void shuffle_guesses_array(int *arr);
void build_guesses_array(int *arr);

typedef struct Node
{
    void *value;
    struct Node *next;
} node_t;

typedef struct LinkedList
{
    int size;
    node_t *head;
    node_t *tail;
} linked_list_t;

int ll_init(linked_list_t **l);
int ll_push(linked_list_t **l, node_t *node);
// void ll_remove(linked_list_t **l, void *value);
void ll_free(linked_list_t **l);

#endif