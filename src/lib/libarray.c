#include <stdio.h>
#include <stdlib.h>

#include "libarray.h"

void shuffle_guesses_array(int *arr)
{
    for (int i = 0; i < GUESSES_SIZE - 1; i++)
    {
        size_t j = i + rand() / (RAND_MAX / (GUESSES_SIZE - i) + 1);
        int t = arr[j];
        arr[j] = arr[i];
        arr[i] = t;
    }
}

void build_guesses_array(int *arr)
{
    for (int i = 0; i < GUESSES_SIZE; i++)
    {
        arr[i] = i + 1;
    }
}

int ll_init(linked_list_t **l)
{
    *l = malloc(sizeof(linked_list_t));
    if (*l == NULL)
    {
        return -1;
    }

    (*l)->size = 0;
    (*l)->head = NULL;
    (*l)->tail = NULL;
    return 0;
}

int ll_push(linked_list_t **l, node_t *node)
{
    if (node == NULL)
    {
        return (*l)->size;
    }

    if ((*l)->head == NULL)
    {
        (*l)->head = node;
        (*l)->tail = node;
        (*l)->size = 1;
        return (*l)->size;
    }

    (*l)->tail->next = node;
    (*l)->tail = node;
    (*l)->size += 1;
    return (*l)->size;
}

// void *ll_remove(linked_list_t **t, void *value)
// {
//     if ((*t)->size == 0)
//         return;

//     node_t *aux;
//     aux = (*t)->head;
//     while (aux->next != value)
//     {
//         aux = aux->next;
//     }

//     aux->next = aux->next->next;
//     free(aux->next);
//     (*t)
//                 void *value = NULL;
//     node_t *tmp = NULL;
//     value = (*t)->head->value;
//     tmp = (*t)->head;
//     (*t)->head = (*t)->head->next;
//     (*t)->size -= 1;
//     free(tmp);
//     return value;
// }

void ll_free(linked_list_t **t)
{
    if (*t == NULL)
        return;

    while ((*t)->head != NULL)
    {
        node_t *tmp = (*t)->head;
        (*t)->head = (*t)->head->next;
        if (tmp->value != NULL)
        {
            free(tmp->value);
        }

        free(tmp);
    }

    if ((*t)->tail != NULL)
    {
        free((*t)->tail);
    }

    free((*t));
}