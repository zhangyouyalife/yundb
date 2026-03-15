#ifndef _BLOCK_SEQ_
#define _BLOCK_SEQ_

#include <stdint.h>

struct blk_record_ref {
    uint32_t    bn;
    uint16_t    tn;
};

struct __attribute__((packed)) blk_tuple
{
    uint16_t    off;
    int16_t     sz;
};

struct __attribute__((packed)) blk_tuple_node
{
    struct blk_tuple tuple;
    struct blk_record_ref next;
};

struct blk_record_ref NULL_TUPLE_REF;

#endif

