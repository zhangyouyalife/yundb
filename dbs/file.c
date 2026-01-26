#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file.h"
#include "block.h"

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

void f_binit(char bd[BLK_SZ])
{
    struct dbf_blkhdr *h;
    
    h = (struct dbf_blkhdr *) bd;
    h->nrec = 0;
    h->free = BLK_SZ;
}

void f_nr(struct dbf *f, char *r, int size)
{
    char *b, *p;
    int i;

    if ( (b = malloc(BLK_SZ)) == 0 )
    {
        perror("f_nr malloc failed");
        exit(EC_M);
    }

    for (i = 1; i < f->hdr->blks; i++) {
        f_rb(f, i, b);
        p = b_nr(b, size);
        if (p != 0)
        {
            memcpy(p, r, size);
            f_wb(f, i, b);
            return;
        }
    }

    /* alloc new block */
    f_binit(b);
    p = b_nr(b, size);
    memcpy(p, r, size);
    f_wb(f, i, b);
    f->hdr->blks++;

    free(b);
}

char *b_nr(char *block, int size)
{
    int free;
    struct dbf_blkhdr *bh;

    bh = (struct dbf_blkhdr *) block;

    free = bh->free 
        - sizeof(struct dbf_blkhdr)
        - sizeof(struct dbf_rec) * bh->nrec;

    if (free < size)
        return NULL;

    bh->rec[bh->nrec].off = bh->free - size;
    bh->rec[bh->nrec].sz = size;
    bh->free -= size;
    bh->nrec++;

    return (block + bh->free);
}

void f_dr(struct dbf_it *it)
{
    struct dbf_blkhdr *h;
    struct dbf_rec *r;
    int off, sz;
    int i;

    h = (struct dbf_blkhdr*) it->blk;
    off = h->rec[it->r].off;
    sz = h->rec[it->r].sz;

    memmove(it->blk + h->free + sz, 
            it->blk + h->free,
            off - h->free);

    for (i = 0; i < h->nrec; i++)
    {
       r = &h->rec[i]; 
       if (r->sz == -1)
           continue;
       if (r->off < off)
           r->off += sz;
    }
    h->free += sz;
    h->rec[it->r].sz = -1;
    
    f_wb(it->f, it->b, it->blk);
}

void f_it(struct dbf *f, struct dbf_it *it)
{
    it->f = f;
    it->blk = 0;
}

char * f_itnext_inblk(struct dbf_it *it)
{
    struct dbf_rec *r;
    struct dbf_blkhdr *h;

    h = (struct dbf_blkhdr *) it->blk;

    for ( ; it->r < h->nrec; it->r++)
    {
        r = &h->rec[it->r];
        if (r->sz != -1) {
            return it->blk + h->rec[it->r].off;
        }
    } 

    return 0;
}

char *f_itnext(struct dbf_it *it)
{
    struct dbf_blkhdr *h;
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
    h = (struct dbf_blkhdr *) it->blk;
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
    if (it->blk)
        free(it->blk);
}

void f_strcpy(char *s, struct dbf_va *v, char *r)
{
    strncpy(s, r + v->off, v->len);
    s[v->len] = 0;
}

void f_ur(struct dbf_it *it,  char *r, int newsz)
{
    struct dbf_blkhdr *h;
    struct dbf_rec *rec;
    int off, sz, newoff;
    int i;

    h = (struct dbf_blkhdr*) it->blk;
    off = h->rec[it->r].off;
    sz = h->rec[it->r].sz;
    newoff = off + sz - newsz;

    memmove(it->blk + h->free + sz - newsz, 
            it->blk + h->free,
            off - h->free);
    memcpy(it->blk + newoff, r, newsz);

    for (i = 0; i < h->nrec; i++)
    {
       rec = &h->rec[i]; 
       if (rec->sz == -1)
           continue;
       if (rec->off < off)
           rec->off += (sz - newsz);
    }
    h->free += (sz - newsz);
    h->rec[it->r].off = newoff;
    h->rec[it->r].sz = newsz;
    
    f_wb(it->f, it->b, it->blk);
}

