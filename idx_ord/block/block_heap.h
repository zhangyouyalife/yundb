#ifndef _BLOCK_HEAP_
#define _BLOCK_HEAP_

#include <stdint.h>

struct blk_ops blk_ops_heap_file_header;

struct blk_ops blk_ops_heap_file_data;

struct __attribute__((packed)) blk_heap_file_header
{
    uint8_t     type;
    uint32_t    blocks;
};

struct __attribute__((packed)) blk_heap_file_data_entry
{
    uint16_t    off;
    int16_t     sz;
};

struct __attribute__((packed)) blk_heap_file_data_header
{
    uint8_t             type;
    uint16_t            nentry;
    uint16_t            free;
    struct blk_heap_file_data_entry   entries[0]; 
};


#endif

