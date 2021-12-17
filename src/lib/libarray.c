#include <stdio.h>
#include <stdlib.h>

#include "libarray.h"

int ll_init(linked_list_t **l)
{
    *l = malloc(sizeof(linked_list_t));
    if (*l == NULL)
    {
        return -1;
    }

    (*l)->size = 0;
    (*l)->head = NULL;
    return 0;
}

int ll_insert(linked_list_t **l, void *value)
{
    node_t *new, *temp;
    new = (node_t *)malloc(sizeof(node_t));
    if (new == NULL)
    {
        return -1;
    }
    new->value = value;
    new->next = NULL;
    temp = (*l)->head;
    printf("inserted0\n");
    if (temp == NULL)
    {
        temp = new;
        printf("inserted1\n");
    }
    else
    {
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = new;
        printf("inserted2\n");
    }
    (*l)->size = (*l)->size + 1;
    printf("inserted3, size: %d\n", (*l)->size);
    return 0;
}

int ll_delete_value(linked_list_t **l, void *value)
{
    if ((*l)->head == NULL)
    {
        return -1;
    }

    node_t *cur = (*l)->head;
    node_t *prev = NULL;
    while (cur->value != value)
    {
        prev = cur;
        cur = cur->next;
    }

    if (prev != NULL)
    {
        prev->next = cur->next;
    }
    free(cur);
    (*l)->size -= 1;
    return 0;
}


int ll_free(linked_list_t **l)
{
    if (*l == NULL)
        return -1;

    while ((*l)->head != NULL)
    {
        node_t *temp = (*l)->head;
        (*l)->head = (*l)->head->next;
        if (temp->value != NULL)
        {
            free(temp->value);
        }

        free(temp);
    }

    free((*l));
    return 0;
}