#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file_order.h"
#include "block/block.h"
#include "block/block_seq.h"
#include "tuple.h"
#include "buf.h"

void f_crt_order(struct dbf *f, char filename[], uint8_t type)
{
    int fd;
    struct dbf_hdr_order *h;

    if ((fd = open(filename, O_CREAT|O_TRUNC|O_RDWR, 0644)) < 0)
    {
        perror("f_crt_heap open failed");
        exit(EC_IO);
    }

    f->fd = fd;
    f->blk0 = b_get(fd, 0);
    f->hdr = (struct dbf_hdr *) f->blk0;
    h = (struct dbf_hdr_order *) f->hdr;
    h->type = FT_ORDER;
    h->blks = 1;
    h->overflow_blk = INVALID_DATA_BLK;
    h->first.bn = INVALID_DATA_BLK;
    h->first.tn = INVALID_TUPLE_NO;
    
    SET_DIRTY(B_BUF(f->blk0));
    b_pin(f->blk0);

    b_put(f->blk0);
}

void f_close_order(struct dbf *f)
{
    if ( -1 == close(f->fd))
    {
        perror("f_close close failed");
        exit(EC_IO);
    }
}

/*
 * precondition:
 *  f is opened
 *  bn is valid
 *  tn is valid
 * 
 * return
 *  the data of tuple at (f, bn, tn)
 */
void f_gr(struct blk_tuple_node *t, struct dbf *f, uint32_t bn, uint16_t tn)
{
    char *b;
    struct blk_hdr *bh;
    struct blk_tuple_node *tp; 

    b = b_get(f->fd, bn);

    bh = (struct blk_hdr *) b;
    tp = (struct blk_tuple_node *) bh->tuples;
    tp += tn;

    memcpy(t, tp, sizeof(*t));

    b_put(b);
}

int blk_nt_generic(char *blk, int ta_sz, int size)
{
    int free, hsz;
    int tn;
    struct blk_hdr *bh;
    struct blk_tuple *t;

    bh = (struct blk_hdr *) blk;

    hsz = sizeof(struct blk_hdr) + ta_sz * bh->ntuple;

    free = bh->free - hsz;

    if (free < size + ta_sz)
        return -1;

    tn = bh->ntuple;
    t = (struct blk_tuple *)(blk + hsz);
    t->off = bh->free - size;
    t->sz = size;

    bh->free -= size;
    bh->ntuple++;

    return tn;
}

#define FNR_EXIST 1

int f_nr_order(struct dbf *f, char *r, int size, int (*cmp)(char*, char*))
{
    char *b, bn;
    int i, tn;
    struct dbf_hdr_order *h;

    int res;
    char *t;
    struct blk_tuple_node node, *qnode, *pnode;
    struct blk_tuple_ref p, q;

    h = (struct dbf_hdr_order *) f->hdr;

    /* loop invariant: q refers to null  or p = next(q) */
    memcpy(&q, &NULL_TUPLE_REF, sizeof(q));
    memcpy(&p, &h->first, sizeof(p));
    res = 1;
    while (res > 0)
    {
        if (p.bn == NULL_TUPLE_REF.bn)
        {
            /* all tuple < null tuple */
            res = -1;
        }
        else
        {
            f_gr(&node, f, p.bn, p.tn);

            b = b_get(f->fd, p.bn);
            t = b + node.tuple.off;
            res = cmp(r, t);
            b_put(b);

            if (res > 0)
            {
                /* greater, check next */
                memcpy(&q, &p, sizeof(q));
                memcpy(&p, &node.next, sizeof(p));
            }
        }
    }

    if (res == 0)
    {
        return FNR_EXIST;
    }

    /* now, res <= 0 */
    if (q.bn == NULL_TUPLE_REF.bn)
    {
        /* as first tuple */
        if (p.bn == NULL_TUPLE_REF.bn)
        {
            /* alloc new blk */
            bn = h->blks;

            b = b_get(f->fd, bn);
            blk_init(b);
            b_put(b);

            h->blks++;
            SET_DIRTY(B_BUF(f->blk0));
        }
        else
        {
            bn = p.bn;

        }
            
        b = b_get(f->fd, bn);
        tn = blk_nt_generic(b, sizeof(struct blk_tuple_node), size);

        if (tn == -1)
        {
            b_put(b);

            /* alloc new blk */
            bn = h->blks;

            b = b_get(f->fd, bn);
            blk_init(b);
            tn = blk_nt_generic(b, sizeof(struct blk_tuple_node), size);

            h->blks++;
            SET_DIRTY(B_BUF(f->blk0));
        }

        pnode = (struct blk_tuple_node *)((struct blk_hdr *)b)->tuples + tn;
        memcpy(b + pnode->tuple.off, r, size);
        pnode->next.bn = p.bn;
        pnode->next.tn = p.tn;
        SET_DIRTY(B_BUF(b));
        b_put(b);

        h->first.bn = bn;
        h->first.tn = tn;
        SET_DIRTY(B_BUF(f->blk0));
    }
    else
    {
        if (q.bn == p.bn)
        {
            /* in the same blk */
            bn = q.bn;
            b = b_get(f->fd, bn);
            tn = blk_nt_generic(b, sizeof(struct blk_tuple_node), size);

            if (tn == -1)
            {
                /* check overflow block */
                bn = h->overflow_blk;
                if (bn != INVALID_DATA_BLK)
                {
                    tn = blk_nt_generic(b, sizeof(struct blk_tuple_node), size);
                }
            }

            if (tn == -1)
            {
                /* alloc new overflow block */
                bn = h->blks;

                b = b_get(f->fd, bn);
                blk_init(b);
                b_put(b);

                h->blks++;
                h->overflow_blk = bn;
                SET_DIRTY(B_BUF(f->blk0));
            }

            pnode = (struct blk_tuple_node *)((struct blk_hdr *)b)->tuples + tn;
            memcpy(b + pnode->tuple.off, r, size);

            pnode->next.bn = p.bn;
            pnode->next.tn = p.tn;
            SET_DIRTY(B_BUF(b));
            b_put(b);

            b = b_get(f->fd, q.bn);
            pnode = (struct blk_tuple_node *)((struct blk_hdr *)b)->tuples + q.tn;
            pnode->next.bn = bn;
            pnode->next.tn = tn;
            SET_DIRTY(B_BUF(b));
            b_put(b);
        }
        else
        {
            /* in different blk */

            /* create tuple q's block */
            bn = q.bn;
            b = b_get(f->fd, bn);
            tn = blk_nt_generic(b, sizeof(struct blk_tuple_node), size);

            if (tn == -1)
            {
                b_put(b);

                bn = p.bn;
                b = b_get(f->fd, bn);
                tn = blk_nt_generic(b, sizeof(struct blk_tuple_node), size);
                /* create tuple in p's block */
            }

            if (tn == -1)
            {
                b_put(b);

                /* alloc new blk */
                bn = h->blks;

                b = b_get(f->fd, bn);
                blk_init(b);
                tn = blk_nt_generic(b, sizeof(struct blk_tuple_node), size);

                h->blks++;
                SET_DIRTY(B_BUF(f->blk0));
            }

            pnode = (struct blk_tuple_node *)((struct blk_hdr *)b)->tuples + tn;
            memcpy(b + pnode->tuple.off, r, size);
            pnode->next.bn = p.bn;
            pnode->next.tn = p.tn;
            SET_DIRTY(B_BUF(b));
            b_put(b);


            b = b_get(f->fd, q.bn);
            pnode = (struct blk_tuple_node *)((struct blk_hdr *)b)->tuples + q.tn;
            pnode->next.bn = bn;
            pnode->next.tn = tn;
            SET_DIRTY(B_BUF(b));
            b_put(b);
        }
    }

    return 0;
}

void f_dr_order(struct dbf_it *it)
{
    char *b;

    b = b_get(it->f->fd, it->b);

    blk_dt(b, it->r);
    
    SET_DIRTY(B_BUF(b));
    b_put(b);
}

void f_it_order(struct dbf *f, struct dbf_it *it)
{
    it->f = f;
    it->b = INVALID_DATA_BLK;
    it->r = INVALID_TUPLE_NO;
}


int f_itnext_order(struct dbf_it *it)
{
    struct dbf_hdr_order *h;
    struct blk_tuple_node *n;

    char *b;

    h = (struct dbf_hdr_order *) it->f->hdr;

    if (it->b == INVALID_DATA_BLK)
    {
        /* before first */
        it->b = h->first.bn;
        it->r = h->first.tn;
    }
    else
    {
        b = b_get(it->f->fd, it->b);
        n = (struct blk_tuple_node *) blk_gt(b, it->r, sizeof(struct blk_tuple_node));

        it->b = n->next.bn;
        it->r = n->next.tn;

        b_put(b);
    }

    return it->b != INVALID_DATA_BLK;
}

void f_itfree_order(struct dbf_it *it)
{
}

void f_ur_order(struct dbf_it *it,  char *r, int newsz)
{
}

