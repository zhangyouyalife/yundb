#include <string.h>
#include <stdio.h>

#include "buf.h"

void b_init()
{
    int i;

    memset(buffer, 0, sizeof(buffer));

    for (i = 0; i < BUF_SZ; i++)
    {
        B_SET_EMPTY(&buffer[i]);
    }
}

void b_sync()
{
    int i;
    struct buf* b;

    for (i = 0; i < BUF_SZ; i++)
    {
        b = &buffer[i];

        if (B_EMPTY(b))
            continue;

        if (!IS_DIRTY(b))
            continue;

        b_fw(B_BLK(b));
    }
}

void b_info()
{
    int i;
    struct buf* b;

    puts("buffer:");
    for (i = 0; i < BUF_SZ; i++)
    {
        b = &buffer[i];

        printf("%d:(%d,%d,%c)\n",
                i, 
                b->b, 
                b->fd,
                (IS_DIRTY(b) ? 'D' : ' '));
    }
}

struct buf *b_fill(struct buf *p, int fd, int b)
{
    p->fd = fd;
    p->b = b;
    p->ref = 1;
    p->flag = 0;
    if (b < sf_blks(fd))
    {
        sf_rb(fd, b, B_BLK(p));
    } else {
        /* new blok */
        SET_DIRTY(p);
        bzero(B_BLK(p), BLK_SZ);
    }
    return p;
}

char *b_get(int fd, int b)
{
    struct buf *p;
    int i;

    /* found buf */
    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        
        if (p->fd == fd && p->b == b)
        {
            p->ref++;
            return B_BLK(p);
        }
    }

    /* found empty slot */
    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        
        if (B_EMPTY(p))
        {
            return B_BLK(b_fill(p, fd, b));
        }
    }

    /* found a non-dirty buf */
    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        if (p->ref == 0 && !IS_DIRTY(p))
        {
            return B_BLK(b_fill(p, fd, b));
        }
    }

    /* found a dirty buf */
    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        if (p->ref == 0 && IS_DIRTY(p))
        {
            sf_wb(p->fd, p->b, B_BLK(p));

            return B_BLK(b_fill(p, fd, b));
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
    printf("b_fw: %d, %d\n", b->fd, b->b);

    if (IS_DIRTY(b))
    {
        sf_wb(b->fd, b->b, blk);
        CLR_DIRTY(b);
    }
}

void b_clearfd(int fd)
{
    int i;
    struct buf *p;

    for (i = 0; i < BUF_SZ; i++)
    {
        p = &buffer[i];
        
        if (p->fd == fd)
        {
            b_fw(B_BLK(p));
            B_SET_EMPTY(p);
        }
    }
}
