#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file_heap.h"
#include "tuple.h"

void f_bs(struct dbf *f, int bn)
{
    f_bs_heap(f, bn);
}

void f_wb(struct dbf *f, int bn, char bd[BLK_SZ])
{
    f_wb_heap(f, bn, bd);
}

void f_rb(struct dbf *f, int bn, char bd[BLK_SZ])
{
    f_rb_heap(f, bn, bd);
}

void f_crt(struct dbf *f, char filename[], uint8_t type)
{

    if (type == FT_HEAP)
    {
        f_crt_heap(f, filename, type);
    }
    else
    {
        perror("f_crt: file type not supported");
        exit(EC_NOT_SUPPORTED);
    }
}

void f_open(struct dbf *f, char filename[])
{
    f_open_heap(f, filename);
}

void f_close(struct dbf *f)
{
    f_close_heap(f);
}

void f_nr(struct dbf *f, char *r, int size)
{
    if (f->hdr->type == FT_HEAP)
    {
        f_nr_heap(f, r, size);
    }
    else
    {
        perror("f_nr: file type not supported");
        exit(EC_NOT_SUPPORTED);
    }
}


void f_dr(struct dbf_it *it)
{
    if (it->f->hdr->type == FT_HEAP)
    {
        f_dr_heap(it);
    }
    else
    {
        perror("f_dr: file type not supported");
        exit(EC_NOT_SUPPORTED);
    }
}

void f_it(struct dbf *f, struct dbf_it *it)
{
    f_it_heap(f, it);
}


char *f_itnext(struct dbf_it *it)
{
    return f_itnext_heap(it);
}

void f_itfree(struct dbf_it *it)
{
    f_itfree_heap(it);
}

void f_ur(struct dbf_it *it,  char *r, int newsz)
{
    f_ur_heap(it, r, newsz);
}

