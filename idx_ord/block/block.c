#include "block/block.h"
#include "block/block_heap.h"

struct blk_ops* block_ops_tab[BT_TABLE_SZ];

struct blk_ops blk_ops_null = {0, 0, 0, 0, 0};

int
blk_types_init()
{
    block_ops_tab[0] = &blk_ops_null;
    block_ops_tab[BT_HEAP_FILE_HEADER] = &blk_ops_heap_file_header;
    block_ops_tab[BT_HEAP_FILE_DATA] = &blk_ops_heap_file_data;

    return 0;
}


int
blk_init(char *blk, int type)
{
    struct  blk_header  *h;

    h = (struct blk_header*) blk;

    if (block_ops_tab[type] 
            && block_ops_tab[type]->blk_init)
    {
        return block_ops_tab[type]->blk_init(blk, type);
    }
    else
    {
        return BLK_OPS_NOT_SUPPORTED;
    }
}

int
blk_entry_create(char *blk, int sz)
{
    struct  blk_header  *h;

    h = (struct blk_header*) blk;

    if (block_ops_tab[h->type] 
            && block_ops_tab[h->type]->blk_entry_create)
    {
        return block_ops_tab[h->type]->blk_entry_create(blk, sz);
    }
    else
    {
        return BLK_OPS_NOT_SUPPORTED;
    }
}

int
blk_entry_delete(char *blk, int n)
{
    struct  blk_header  *h;

    h = (struct blk_header*) blk;

    if (block_ops_tab[h->type] 
            && block_ops_tab[h->type]->blk_entry_delete)
    {
        return block_ops_tab[h->type]->blk_entry_delete(blk, n);
    }
    else
    {
        return BLK_OPS_NOT_SUPPORTED;
    }
}

int
blk_entry_update(char *blk, int n, int sz)
{
    struct  blk_header  *h;

    h = (struct blk_header*) blk;

    if (block_ops_tab[h->type] 
            && block_ops_tab[h->type]->blk_entry_update)
    {
        return block_ops_tab[h->type]->blk_entry_update(blk, n, sz);
    }
    else
    {
        return BLK_OPS_NOT_SUPPORTED;
    }
}

char*
blk_entry_get(char *blk, int n)
{
    struct  blk_header  *h;

    h = (struct blk_header*) blk;

    if (block_ops_tab[h->type] 
            && block_ops_tab[h->type]->blk_entry_get)
    {
        return block_ops_tab[h->type]->blk_entry_get(blk, n);
    }
    else
    {
        return 0;
    }
}

char*
blk_record(char *blk, int n)
{
    struct  blk_header  *h;

    h = (struct blk_header*) blk;

    if (block_ops_tab[h->type] 
            && block_ops_tab[h->type]->blk_record)
    {
        return block_ops_tab[h->type]->blk_record(blk, n);
    }
    else
    {
        return 0;
    }
}

