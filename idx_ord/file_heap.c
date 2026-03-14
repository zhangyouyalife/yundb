#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file_heap.h"
#include "tuple.h"
#include "buf.h"

void f_crt_heap(struct dbf *f, char filename[], uint8_t type)
{
    int fd;
    struct dbf_hdr_heap *h;

    if ((fd = open(filename, O_CREAT|O_TRUNC|O_RDWR, 0644)) < 0)
    {
        perror("f_crt_heap open failed");
        exit(EC_IO);
    }

    f->fd = fd;
    f->blk0 = b_get(fd, 0);
    f->hdr = (struct dbf_hdr *) f->blk0;
    h = (struct dbf_hdr_heap *) f->hdr;
    h->type = FT_HEAP;
    h->blks = 1;
    
    SET_DIRTY(B_BUF(f->blk0));
    b_pin(f->blk0);

    b_put(f->blk0);
}

void f_open_heap(struct dbf *f, char filename[])
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

void f_close_heap(struct dbf *f)
{
    if ( -1 == close(f->fd))
    {
        perror("f_close close failed");
        exit(EC_IO);
    }
}

void f_nr_heap(struct dbf *f, char *r, int size)
{
    char *b;
    int i, tn;
    struct dbf_hdr_heap *h;

    h = (struct dbf_hdr_heap *) f->hdr;
    for (i = 1; i < h->blks; i++) {

        b = b_get(f->fd, i);

        tn = blk_nt(b, size);

        if (tn != -1)
        {
            memcpy(b + BLK_GET(b, tn)->off , r, size);
            SET_DIRTY(B_BUF(b));
            b_put(b);
            return;
        }

        b_put(b);
    }

    /* alloc new block */
    b = b_get(f->fd, i);
    blk_init(b);
    tn = blk_nt(b, size);
    memcpy(b + BLK_GET(b, tn)->off , r, size);
    SET_DIRTY(B_BUF(b));
    b_put(b);

    h->blks++;
    SET_DIRTY(B_BUF(f->blk0));
}

void f_dr_heap(struct dbf_it *it)
{
    char *b;

    b = b_get(it->f->fd, it->b);

    blk_dt(b, it->r);

    SET_DIRTY(B_BUF(b));
    b_put(b);
}

void f_it_heap(struct dbf *f, struct dbf_it *it)
{
    it->f = f;
    it->b = 1;
    it->r = -1;
}

int f_itnext_heap(struct dbf_it *it)
{
    char *b;
    struct dbf_hdr_heap *fh;
    struct blk_tuple *bt;
    struct blk_hdr *bh;
    int found;

    fh = (struct dbf_hdr_heap *)it->f->hdr;

    found = 0;
    while (!found && it->b < fh->blks)
    {
        b = b_get(it->f->fd, it->b);

        bh = (struct blk_hdr*) b;
        while (!found && ++(it->r) < bh->ntuple)
        {
            bt = blk_gt(b, it->r, sizeof(struct blk_tuple));

            if (bt && bt->off > 0) {
                found = 1;
            }
        }

        b_put(b);

        if (!found)
        {
            it->b++;
            it->r = -1;
        }
    }

    return found;

}

void f_itfree_heap(struct dbf_it *it)
{
}

void f_ur_heap(struct dbf_it *it,  char *r, int newsz)
{
    struct blk_hdr *h;
    struct blk_tuple *rec;
    int off, sz, newoff;
    int i;
    char *b;

    b = b_get(it->f->fd, it->b);

    h = (struct blk_hdr*) b;
    off = h->tuples[it->r].off;
    sz = h->tuples[it->r].sz;
    newoff = off + sz - newsz;

    memmove(b + h->free + sz - newsz, 
            b + h->free,
            off - h->free);
    memcpy(b + newoff, r, newsz);

    for (i = 0; i < h->ntuple; i++)
    {
       rec = &h->tuples[i]; 
       if (rec->sz == -1)
           continue;
       if (rec->off < off)
           rec->off += (sz - newsz);
    }
    h->free += (sz - newsz);
    h->tuples[it->r].off = newoff;
    h->tuples[it->r].sz = newsz;
    

    SET_DIRTY(B_BUF(b));
    b_put(b);
}

