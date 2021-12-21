#include <stdio.h>
#include <stdlib.h>

#include "libarray.h"

int ll_init(struct linked_list_t **l)
{
    *l = malloc(sizeof(struct linked_list_t));
    if (*l == NULL)
    {
        return -1;
    }

    (*l)->size = 0;
    return 0;
}

int ll_insert(struct linked_list_t **l, void *value)
{
    if ((*l)->head == NULL) { // Insert @ head
        (*l)->head = malloc(sizeof(struct node_t));
        (*l)->head->next = NULL;
        (*l)->head->value = value;
        (*l)->size += 1;
        return 0;
    }

    struct node_t *cur = (*l)->head, *prev;
    while (cur->next != NULL) {
        prev = cur;
        cur = cur->next;
    }

    struct node_t *new;    
    new = (struct node_t*)malloc(sizeof(struct node_t));
	if (new == NULL) {
		return -1;
	}
	new->next = NULL;
	new->value = value;

	cur->next = new;
    (*l)->size += 1;
    return 0;
}

int ll_delete_value(struct linked_list_t **l, void *value)
{
    // Store head node
    struct node_t *temp = (*l)->head, *prev;
 
    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->value == value) {
        (*l)->head = temp->next; // Changed head
        free(temp); // free old head
        (*l)->size -= 1;
        return 0;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->value != value) {
        prev = temp;
        temp = temp->next;
    }
 
    // If key was not present in linked list, this shouldn't happen anyway
    if (temp == NULL)
        return -1;

    // Unlink the node from linked list
    prev->next = temp->next;
    free(temp);
    (*l)->size -= 1;
    return 1;
}


int ll_free(struct linked_list_t **l)
{
    if (*l == NULL)
        return -1;

    while ((*l)->head != NULL)
    {
        struct node_t *temp = (*l)->head;
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