#include <stdio.h>
#include <stdlib.h>

#include "libarray.h"

int ll_insert(struct node_t **head, void *value)
{
    // If the list is empty
    if ((*head) == NULL)
    {
        (*head) = malloc(sizeof(struct node_t));
        (*head)->value = value;
        (*head)->next = NULL;
        return 0;
    }

    struct node_t *cur = (*head);
    while (cur->next != NULL)
    {
        cur = cur->next;
    }

    struct node_t *new;
    new = (struct node_t *)malloc(sizeof(struct node_t));
    if (new == NULL)
    {
        return -1;
    }

    new->next = NULL;
    new->value = value;

    cur->next = new;
    return 0;
}

int ll_delete_value(struct node_t **head, void *value)
{
    struct node_t *temp = *head, *prev;
    // If the head node itself holds the key to be deleted
    if (temp != NULL && temp->value == value)
    {
        (*head) = temp->next; // Changed head
        free(temp);
        return 0;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->value != value)
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list, this shouldn't happen anyway
    if (temp == NULL)
        return -1;

    // Unlink the node from linked list
    prev->next = temp->next;
    free(temp);
    return 1;
}


int ll_free(struct node_t **head)
{
    if ((*head) == NULL)
        return -1;

    struct node_t *tmp;

    while ((*head) != NULL)
    {
        tmp = (*head);
        (*head) = (*head)->next;
        free(tmp);
    }
    return 0;
}

int ll_size(struct node_t *head)
{
    struct node_t *tmp = head;
    int count = 0;

    while (tmp != NULL)
    {
        count += 1;
        tmp = tmp->next;
    }
    return count;
}