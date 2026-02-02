#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file.h"
#include "tuple.h"

void f_bs(struct dbf *f, int bn)
{
    if (-1 == lseek(f->fd, bn * BLK_SZ, SEEK_SET))
    {
        perror("f_bs lseek error");
        exit(EC_IO);
    }
}

void f_wb(struct dbf *f, int bn, char bd[BLK_SZ])
{
    f_bs(f, bn);

    if (BLK_SZ != write(f->fd, bd, BLK_SZ))
    {
        perror("f_wb write error");
        exit(EC_IO);
    }
}

void f_rb(struct dbf *f, int bn, char bd[BLK_SZ])
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

void f_crt(struct dbf *f, char filename[])
{
    int fd;

    if ((fd = open(filename, O_CREAT|O_TRUNC|O_RDWR, 0644)) < 0)
    {
        perror("f_crt open failed");
        exit(EC_IO);
    }

    f->fd = fd;
    f->hdr = (struct dbf_hdr *) f->blk0;
    f->hdr->blks = 1;
    
    f_wb(f, 0, f->blk0);
}

void f_open(struct dbf *f, char filename[])
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

void f_close(struct dbf *f)
{
    f_wb(f, 0, f->blk0);

    if ( -1 == close(f->fd))
    {
        perror("f_close close failed");
        exit(EC_IO);
    }
}

void f_nr(struct dbf *f, char *r, int size)
{
    char *b;
    int i, tn;

    if ( (b = malloc(BLK_SZ)) == 0 )
    {
        perror("f_nr malloc failed");
        exit(EC_M);
    }

    for (i = 1; i < f->hdr->blks; i++) {
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
    f->hdr->blks++;

    free(b);
}

void f_dr(struct dbf_it *it)
{

    blk_dt(it->blk, it->r);
    
    f_wb(it->f, it->b, it->blk);
}

void f_it(struct dbf *f, struct dbf_it *it)
{
    it->f = f;
    it->blk = 0;
}

char * f_itnext_inblk(struct dbf_it *it)
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

char *f_itnext(struct dbf_it *it)
{
    struct blk_hdr *h;
    char *r;

    if (it->blk == 0)
    {
        if (it->f->hdr->blks <= 1)
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
    while (it->b + 1 < it->f->hdr->blks)
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

void f_itfree(struct dbf_it *it)
{
    free(it->blk);
}

void f_ur(struct dbf_it *it,  char *r, int newsz)
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

