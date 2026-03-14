#ifndef _FILE_HEAP_
#define _FILE_HEAP_

#include <stdint.h>

#include "conf.h"
#include "tuple.h"
#include "blk.h"
#include "file.h"

/* Database file header */ 
struct __attribute__((packed)) dbf_hdr_order
{
    uint8_t     type;
    uint32_t    blks;
    uint32_t    overflow_blk;
    struct blk_tuple_ref    first;
};

void f_crt_order(struct dbf *f, char filename[], uint8_t type);
void f_open_order(struct dbf *f, char filename[]);
void f_close_order(struct dbf *f);
int f_nr_order(struct dbf *f, char *r, int size, int (*cmp)(char *, char *));
void f_dr_order(struct dbf_it *it);
void f_ur_heap(struct dbf_it *it, char *r, int size);

/* file iterator */
void f_it_order(struct dbf *f, struct dbf_it *it);
char *f_itnext_order(struct dbf_it *it);
void f_itfree_order(struct dbf_it *it);

#endif

