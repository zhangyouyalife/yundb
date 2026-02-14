#ifndef _FILE_HEAP_
#define _FILE_HEAP_

#include <stdint.h>

#include "conf.h"
#include "tuple.h"
#include "blk.h"
#include "file.h"

/* Database file header */ 
struct __attribute__((packed)) dbf_hdr_heap
{
    uint8_t     type;
    uint32_t    blks;
};

void f_crt_heap(struct dbf *f, char filename[], uint8_t type);
void f_open_heap(struct dbf *f, char filename[]);
void f_close_heap(struct dbf *f);
void f_nr_heap(struct dbf *f, char *r, int size);
void f_dr_heap(struct dbf_it *it);
void f_ur_heap(struct dbf_it *it, char *r, int size);

/* file iterator */
void f_it_heap(struct dbf *f, struct dbf_it *it);
char *f_itnext_heap(struct dbf_it *it);
void f_itfree_heap(struct dbf_it *it);

#endif

