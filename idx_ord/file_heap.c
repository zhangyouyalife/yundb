#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file_heap.h"
#include "tuple.h"
#include "buf.h"
#include "block/block.h"
#include "block/block_heap.h"

void f_crt_heap(struct dbf *f, char filename[], uint8_t type)
{
    int fd;

    if ((fd = open(filename, O_CREAT|O_TRUNC|O_RDWR, 0644)) < 0)
    {
        perror("f_crt_heap open failed");
        exit(EC_IO);
    }

    f->fd = fd;
    f->blk0 = b_get(fd, 0);

    blk_init(f->blk0, BT_HEAP_FILE_HEADER);
    
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

    b_pin(f->blk0);

    b_put(f->blk0);
}

void f_close_heap(struct dbf *f)
{
    if (f->blk0)
    {
        b_unp(f->blk0);
    }

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
    struct blk_heap_file_header *h;

    h = (struct blk_heap_file_header *) f->blk0;
    for (i = 1; i < h->blocks; i++) {

        b = b_get(f->fd, i);

        tn = blk_entry_create(b, size);

        if (tn >= 0)
        {
            memcpy(blk_record(b, tn) , r, size);

            SET_DIRTY(B_BUF(b));
            b_put(b);
            return;
        }

        b_put(b);
    }

    /* alloc new block */
    b = b_get(f->fd, i);

    blk_init(b, BT_HEAP_FILE_DATA);

    tn = blk_entry_create(b, size);
    memcpy(blk_record(b, tn) , r, size);
    SET_DIRTY(B_BUF(b));
    b_put(b);

    h->blocks++;
    SET_DIRTY(B_BUF(f->blk0));
}

void f_dr_heap(struct dbf_it *it)
{
    char *b;

    b = b_get(it->f->fd, it->b);

    blk_entry_delete(b, it->r);

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
    struct blk_heap_file_header *fh;
    struct blk_heap_file_data_entry *bt;
    struct blk_heap_file_data_header *bh;
    int found;

    fh = (struct blk_heap_file_header *)it->f->blk0;

    found = 0;
    while (!found && it->b < fh->blocks)
    {
        b = b_get(it->f->fd, it->b);

        bh = (struct blk_heap_file_data_header*) b;

        while (!found && ++(it->r) < bh->nentry)
        {
            bt = (struct blk_heap_file_data_entry *) blk_entry_get(b, it->r);

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
    char *b;

    b = b_get(it->f->fd, it->b);

    blk_entry_update(b, it->r, newsz);

    memcpy(blk_record(b, it->r), r, newsz);

    SET_DIRTY(B_BUF(b));
    b_put(b);
}

