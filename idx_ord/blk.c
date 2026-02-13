/* blk.c block management */
#include <string.h>

#include "conf.h"
#include "blk.h"

void blk_init(char *blk)
{
    struct blk_hdr *h;
    
    h = (struct blk_hdr *) blk;
    h->ntuple = 0;
    h->free = BLK_SZ;
}

int blk_nt(char *blk, int size)
{
    int free;
    int tn;
    struct blk_hdr *bh;

    bh = (struct blk_hdr *) blk;

    free = bh->free 
        - sizeof(struct blk_hdr)
        - sizeof(struct blk_tuple) * bh->ntuple;

    if (free < size)
        return -1;

    tn = bh->ntuple;
    bh->tuples[tn].off = bh->free - size;
    bh->tuples[tn].sz = size;
    bh->free -= size;
    bh->ntuple++;

    return tn;
}

void blk_dt(char *blk, int tn)
{
    struct blk_hdr *h;
    struct blk_tuple *t, *r;
    int off, sz;
    int i;

    h = (struct blk_hdr *) blk;
    t = BLK_GET(blk, tn);

    off = t->off;
    sz = t->sz;

    /* rearrange storage */
    memmove(blk + h->free + sz, 
            blk + h->free,
            off - h->free);

    /* update tuple offset */
    for (i = 0; i < h->ntuple; i++)
    {
       r = BLK_GET(blk, i); 
       if (BLK_ISNONE(r))
           continue;
       if (r->off < off)
           r->off += sz;
    }
    h->free += sz;
    
    BLK_SETNONE(t);
}

void blk_ut(char *blk, int tn, char *newt, int newsz)
{
    struct blk_hdr *h;
    struct blk_tuple *t;
    int off, sz, newoff;
    int i;

    h = (struct blk_hdr*) blk;

    off = h->tuples[tn].off;
    sz = h->tuples[tn].sz;
    newoff = off + sz - newsz;

    /* expand or shrink stroage */
    memmove(blk + h->free + sz - newsz, 
            blk + h->free,
            off - h->free);
    /* copy new tuple */
    memcpy(blk + newoff, newt, newsz);
    /* update offset of tuples */
    for (i = 0; i < h->ntuple; i++)
    {
       t = BLK_GET(blk, i);
       if (BLK_ISNONE(t))
           continue;
       if (t->off < off)
           t->off += (sz - newsz);
    }
    h->free += (sz - newsz);
    h->tuples[tn].off = newoff;
    h->tuples[tn].sz = newsz;
}

int blk_freespace(char *blk)
{
    int free;
    int tn;
    struct blk_hdr *bh;

    bh = (struct blk_hdr *) blk;

    free = bh->free 
        - sizeof(struct blk_hdr)
        - sizeof(struct blk_tuple) * bh->ntuple;

    return free;
}
