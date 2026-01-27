#ifndef _LIST_
#define _LIST_

struct linkhead
{
    struct linknode *first;
};

struct linknode
{
    void *data;
    void (*freedata)(void *data);
    struct linknode *next;
};

void ll_init(struct linkhead *h);

int ll_empty(struct linkhead *h);

void ll_add(struct linkhead *h, void *data);

void ll_free(struct linkhead *h);

void ll_travel(struct linkhead *h, void (*f)(void *data));

#endif

