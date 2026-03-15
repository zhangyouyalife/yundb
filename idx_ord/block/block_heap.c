#include "conf.h"
#include "block/block_heap.h"
#include "block/block.h"

#include <string.h>

int
heap_file_header_blk_init(char *blk, int type)
{
    struct blk_heap_file_header *h;

    h = (struct blk_heap_file_header *) blk;
    h->type = BT_HEAP_FILE_HEADER;
    h->blocks = 1;
    
    return 0;
}

struct blk_ops blk_ops_heap_file_header = {
    heap_file_header_blk_init,
    0,
    0,
    0,
    0,
    0
};

int
heap_file_data_blk_init(char *blk, int type)
{
    struct blk_heap_file_data_header *h;

    h = (struct blk_heap_file_data_header*) blk;
    h->type = BT_HEAP_FILE_DATA;
    h->nentry = 0;
    h->free = BLK_SZ; 
    
    return 0;
}

int
heap_file_data_blk_entry_create(char *blk, int sz)
{

    int free;
    struct blk_heap_file_data_entry *e;
    struct blk_heap_file_data_header *h;

    h = (struct blk_heap_file_data_header*) blk;

    free = h->free
        - sizeof(struct blk_heap_file_data_header)
        - sizeof(struct blk_heap_file_data_entry) * h->nentry;

    if (free < sz)
        return BLK_OPS_FAIL;


    e = h->entries + h->nentry;
    e->off = h->free - sz;
    e->sz = sz;

    h->free -= sz;
    h->nentry++;
   
    return h->nentry - 1; 
}

int
heap_file_data_blk_entry_delete(char *blk, int n)
{
    struct blk_heap_file_data_header *h;
    struct blk_heap_file_data_entry *e, *p;
    int off, sz;
    int i;

    h = (struct blk_heap_file_data_header *) blk;

    if (n >= h->nentry)
        return BLK_OPS_FAIL;

    e = h->entries + n;
    off = e->off;
    sz = e->sz;

    /* rearrange storage */
    memmove(blk + h->free + sz, 
            blk + h->free,
            off - h->free);

    /* update tuple offset */
    for (i = 0; i < h->nentry; i++)
    {
       p = h->entries + i;
       if (p->sz == -1)
           continue;
       if (p->off < off)
           p->off += sz;
    }

    h->free += sz;

    e->off = 0;
    e->sz = -1;

    return 0;
}

int
heap_file_data_blk_entry_update(char *blk, int n, int newsz)
{
    struct blk_heap_file_data_header *h;
    struct blk_heap_file_data_entry *e, *p;
    int diff, off, sz, newoff;
    int i;

    h = (struct blk_heap_file_data_header*) blk;

    if (n >= h->nentry)
        return BLK_OPS_FAIL;

    e = &h->entries[n];
    off = e->off;
    sz = e->sz;

    diff = sz - newsz;
    newoff = off + diff;

    /* expand or shrink stroage */
    memmove(blk + h->free + diff, 
            blk + h->free,
            off - h->free);

    /* update offset of tuples */
    for (i = 0; i < h->nentry; i++)
    {
       p = h->entries + i;
       if (p->sz == -1)
           continue;
       if (p->off < off)
           p->off += diff;
    }

    h->free += diff;
    e->off = newoff;
    e->sz = newsz;

    return 0;
}

char*
heap_file_data_blk_entry_get(char *blk, int n)
{
    struct blk_heap_file_data_header *h;
    struct blk_heap_file_data_entry *e, *p;
    int i;

    h = (struct blk_heap_file_data_header*) blk;

    if (n >= h->nentry)
        return 0;

    return (char *) &h->entries[n];
}

char*
heap_file_data_blk_record(char *blk, int n)
{
    struct blk_heap_file_data_header *h;
    struct blk_heap_file_data_entry *e, *p;
    int i;

    e = (struct blk_heap_file_data_entry *) heap_file_data_blk_entry_get(blk, n);

    if (e)
        return blk + e->off;
    else
        return 0;
}

struct blk_ops blk_ops_heap_file_data = {
    heap_file_data_blk_init,
    heap_file_data_blk_entry_create,
    heap_file_data_blk_entry_delete,
    heap_file_data_blk_entry_update,
    heap_file_data_blk_entry_get,
    heap_file_data_blk_record
};

