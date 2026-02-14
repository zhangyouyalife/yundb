#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file_heap.h"
#include "tuple.h"

void f_bs_heap(struct dbf *f, int bn)
{
    if (-1 == lseek(f->fd, bn * BLK_SZ, SEEK_SET))
    {
        perror("f_bs lseek error");
        exit(EC_IO);
    }
}

void f_wb_heap(struct dbf *f, int bn, char bd[BLK_SZ])
{
    f_bs(f, bn);

    if (BLK_SZ != write(f->fd, bd, BLK_SZ))
    {
        perror("f_wb write error");
        exit(EC_IO);
    }
}

void f_rb_heap(struct dbf *f, int bn, char bd[BLK_SZ])
{
    int r;
    char *p;

    f_bs(f, bn);

    p = bd;
    while ((r = read(f->fd, p, bd + BLK_SZ - p)) > 0)
        p += r;

    if (p - bd != BLK_SZ)
    {
        perror("f_rb read error");
        exit(EC_IO);
    }
}

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
    bzero(f->blk0, BLK_SZ);
    f->hdr = (struct dbf_hdr *) f->blk0;
    h = (struct dbf_hdr_heap *) f->hdr;
    h->type = FT_HEAP;
    h->blks = 1;
    
    f_wb(f, 0, f->blk0);
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
    f->hdr = (struct dbf_hdr *) f->blk0;

    f_rb(f, 0, f->blk0);
}

void f_close_heap(struct dbf *f)
{
    f_wb(f, 0, f->blk0);

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

    if ( (b = malloc(BLK_SZ)) == 0 )
    {
        perror("f_nr malloc failed");
        exit(EC_M);
    }

    h = (struct dbf_hdr_heap *) f->hdr;
    for (i = 1; i < h->blks; i++) {
        f_rb(f, i, b);
        tn = blk_nt(b, size);
        if (tn != -1)
        {
            memcpy(b + BLK_GET(b, tn)->off , r, size);
            f_wb(f, i, b);
            return;
        }
    }

    /* alloc new block */
    blk_init(b);
    tn = blk_nt(b, size);
    memcpy(b + BLK_GET(b, tn)->off , r, size);
    f_wb(f, i, b);
    h->blks++;

    free(b);
}

void f_dr_heap(struct dbf_it *it)
{

    blk_dt(it->blk, it->r);
    
    f_wb(it->f, it->b, it->blk);
}

void f_it_heap(struct dbf *f, struct dbf_it *it)
{
    it->f = f;
    it->blk = 0;
}

static char * f_itnext_inblk(struct dbf_it *it)
{
    struct blk_tuple *r;
    struct blk_hdr *h;

    h = (struct blk_hdr *) it->blk;

    for ( ; it->r < h->ntuple; it->r++)
    {
        r = &h->tuples[it->r];
        if (r->sz != -1) {
            return it->blk + h->tuples[it->r].off;
        }
    } 

    return 0;
}

char *f_itnext_heap(struct dbf_it *it)
{
    struct blk_hdr *h;
    char *r;
    struct dbf_hdr_heap *fh;

    fh = (struct dbf_hdr_heap *)it->f->hdr;
    if (it->blk == 0)
    {
        if (fh->blks <= 1)
            /* no data block */
            return 0;

        /* init */
        it->blk = malloc(BLK_SZ);
        if (it->blk == 0)
        {
            perror("f_itnext malloc failed");
            exit(EC_M);
        }

        it->b = 1;
        it->r = -1;
        f_rb(it->f, it->b, it->blk); 
    }
    h = (struct blk_hdr *) it->blk;
    it->r++;

    /* return record in curent block */
    r = f_itnext_inblk(it);
    if (r != 0)
        return r;

    /* read next blocks */
    while (it->b + 1 < fh->blks)
    {
        f_rb(it->f, ++(it->b), it->blk);
        it->r = 0;
        r = f_itnext_inblk(it);
        if (r != 0)
            return r;
    }

    /* no more record */
    return 0;
}

void f_itfree_heap(struct dbf_it *it)
{
    free(it->blk);
}

void f_ur_heap(struct dbf_it *it,  char *r, int newsz)
{
    struct blk_hdr *h;
    struct blk_tuple *rec;
    int off, sz, newoff;
    int i;

    h = (struct blk_hdr*) it->blk;
    off = h->tuples[it->r].off;
    sz = h->tuples[it->r].sz;
    newoff = off + sz - newsz;

    memmove(it->blk + h->free + sz - newsz, 
            it->blk + h->free,
            off - h->free);
    memcpy(it->blk + newoff, r, newsz);

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
    
    f_wb(it->f, it->b, it->blk);
}

