#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file_heap.h"
#include "file_order.h"
#include "tuple.h"
#include "buf.h"

void f_crt(struct dbf *f, char filename[], uint8_t type)
{

    switch (type)
    {
        case FT_HEAP:
            f_crt_heap(f, filename, type);
            break;
        case FT_ORDER:
            f_crt_order(f, filename, type);
            break;
        default:
            perror("f_crt: file type not supported");
            exit(EC_NOT_SUPPORTED);
    }
}

static void f_open_generic(struct dbf *f, char filename[])
{
    int fd;

    if ((fd = open(filename, O_RDWR)) < 0)
    {
        perror("f_open open failed");
        exit(EC_IO);
    }

    f->fd = fd;
    f->blk0 = b_get(fd, 0);
    f->hdr = (struct dbf_hdr *) f->blk0;

    b_pin(f->blk0);

    b_put(f->blk0);
}

void f_open(struct dbf *f, char filename[])
{
    f_open_generic(f, filename);
}

void f_close(struct dbf *f)
{
    switch (f->hdr->type)
    {
        case FT_HEAP:
            f_close_heap(f);
            break;
        case FT_ORDER:
            f_close_order(f);
            break;
        default:
            perror("f_close: file type not supported");
            exit(EC_NOT_SUPPORTED);

    }
}

void f_nr(struct dbf *f, char *r, int size, int (*cmp)(char *, char*))
{
    switch (f->hdr->type)
    {
        case FT_HEAP:
            f_nr_heap(f, r, size);
            break;
        case FT_ORDER:
            f_nr_order(f, r, size, cmp);
            break;
        default:
            perror("f_nr: file type not supported");
            exit(EC_NOT_SUPPORTED);

    }
}


void f_dr(struct dbf_it *it)
{
    switch (it->f->hdr->type)
    {
        case FT_HEAP:
            f_dr_heap(it);
            break;
        case FT_ORDER:
            f_dr_order(it);
            break;
        default:
            perror("f_dr: file type not supported");
            exit(EC_NOT_SUPPORTED);

    }
}

void f_it(struct dbf *f, struct dbf_it *it)
{
    switch (f->hdr->type)
    {
        case FT_HEAP:
            f_it_heap(f, it);
            break;
        case FT_ORDER:
            f_it_order(f, it);
            break;
        default:
            perror("f_it: file type not supported");
            exit(EC_NOT_SUPPORTED);

    }
}


int f_itnext(struct dbf_it *it)
{
    switch (it->f->hdr->type)
    {
        case FT_HEAP:
            return f_itnext_heap(it);
        case FT_ORDER:
            return f_itnext_order(it);
        default:
            perror("f_itnext: file type not supported");
            exit(EC_NOT_SUPPORTED);

    }
}

void f_itfree(struct dbf_it *it)
{
    switch (it->f->hdr->type)
    {
        case FT_HEAP:
            f_itfree_heap(it);
            break;
        case FT_ORDER:
            f_itfree_order(it);
            break;
        default:
            perror("f_itfree: file type not supported");
            exit(EC_NOT_SUPPORTED);

    }
}

void f_ur(struct dbf_it *it,  char *r, int newsz)
{
    f_ur_heap(it, r, newsz);
}

