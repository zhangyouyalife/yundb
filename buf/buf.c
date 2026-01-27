#include <string.h>

#include "buf.h"

void b_init()
{
}

void b_sync()
{
    int i;
}

struct buf *b_fill(struct buf *p, struct dbf *f, int b)
{
    p->f = f;
    p->b = b;
    p->ref = 1;
    p->flag = 0;
    if (b < f->hdr->blks)
    {
        f_rb(f, b, B_BLK(p));
    } else {
        /* new blok */
        SET_DIRTY(p);
        bzero(B_BLK(p), BLK_SZ);
    }
    return p;
}

char *b_get(struct dbf *f, int b)
{
    struct buf *p;
    int i;

    /* found buf */
    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        
        if (p->f == f && p->b == b)
        {
            p->ref++;
            B_BLK(p);
        }
    }

    /* found empty slot */
    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        
        if (p->f == 0)
        {
            return B_BLK(b_fill(p, f, b));
        }
    }

    /* found a non-dirty buf */
    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        if (p->ref == 0 && !IS_DIRTY(p))
        {
            return B_BLK(b_fill(p, f, b));
        }
    }

    /* found a dirty buf */
    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        if (p->ref == 0 && IS_DIRTY(p))
        {
            f_wb(p->f, p->b, B_BLK(p));

            return B_BLK(b_fill(p, f, b));
        }
    }

    return 0;
}

void b_put(char *b)
{
    B_BUF(b)->ref--;
}

void b_pin(char *b)
{
    B_BUF(b)->ref++;
}

void b_unp(char *b)
{
    B_BUF(b)->ref--;
}

void b_fw(char *blk)
{
    struct buf *b;

    b = B_BUF(blk);

    if (IS_DIRTY(b))
    {
        f_wb(b->f, b->b, blk);
        CLR_DIRTY(b);
    }
}
