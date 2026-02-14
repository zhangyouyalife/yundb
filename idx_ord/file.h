#ifndef _FILE_
#define _FILE_

#include <stdint.h>

#include "conf.h"
#include "tuple.h"
#include "blk.h"

#define FT_HEAP 1

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
    char            blk0[BLK_SZ];   
};

struct dbf_it
{
    struct dbf *f;
    uint16_t b;
    int16_t r;
    char *blk;
};

void f_bs(struct dbf *f, int bn);
void f_wb(struct dbf *f, int bn, char bd[BLK_SZ]);
void f_rb(struct dbf *f, int bn, char bd[BLK_SZ]);

void f_crt(struct dbf *f, char filename[], uint8_t type);
void f_open(struct dbf *f, char filename[]);
void f_close(struct dbf *f);
void f_nr(struct dbf *f, char *r, int size);
void f_dr(struct dbf_it *it);
void f_ur(struct dbf_it *it, char *r, int size);

/* file iterator */
void f_it(struct dbf *f, struct dbf_it *it);
char *f_itnext(struct dbf_it *it);
void f_itfree(struct dbf_it *it);

#endif

