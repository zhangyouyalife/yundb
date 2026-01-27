#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "exitcode.h"

void ll_init(struct linkhead *h)
{
    h->first = 0;
}

int ll_empty(struct linkhead *h)
{
    return (h->first == 0);
}

void ll_add(struct linkhead *h, void *data)
{
    struct linknode *n;

    if ( (n = malloc(sizeof(struct linknode))) == 0)
    {
        perror("ll_add malloc");
        exit(EC_M);
    }

    n->data = data;
    n->next = h->first;

    h->first = n;
}

void ll_free(struct linkhead *h)
{
    struct linknode *n;

    while (h->first != 0)
    {
        n = h->first;
        h->first = n->next;    
        free(n);
    }
}

void ll_travel(struct linkhead *h, void (*f)(void *data))
{
    struct linknode *n;

    n = h->first;
    while (n != 0)
    {
        f(n->data);
        n = n->next;
    }
}
