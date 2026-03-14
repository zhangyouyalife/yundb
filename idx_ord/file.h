#ifndef _FILE_
#define _FILE_

#include <stdint.h>

#include "conf.h"
#include "tuple.h"
#include "blk.h"

#define INVALID_DATA_BLK    0
#define INVALID_TUPLE_NO    (-1)

#define FT_HEAP     1
#define FT_ORDER    2

/* Database file header */ 
struct __attribute__((packed)) dbf_hdr
{
    uint8_t type;
    uint32_t blks;
};

struct dbf
{
    int                  fd;
    struct dbf_hdr       *hdr;
    char            *blk0;   
};

struct dbf_it
{
    struct dbf *f;
    uint16_t b;
    int16_t r;
};


void f_crt(struct dbf *f, char filename[], uint8_t type);
void f_open(struct dbf *f, char filename[]);
void f_close(struct dbf *f);
void f_nr(struct dbf *f, char *r, int size, int (*cmp)(char *, char *));
void f_dr(struct dbf_it *it);
void f_ur(struct dbf_it *it, char *r, int size);

char* f_gt(struct dbf_it *it);
void f_ft(char *t);

/* file iterator */
void f_it(struct dbf *f, struct dbf_it *it);
int f_itnext(struct dbf_it *it);
void f_itfree(struct dbf_it *it);

#endif

