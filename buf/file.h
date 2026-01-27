#ifndef _FILE_
#define _FILE_

#include <stdint.h>

#include "conf.h"

/* Database file header */ 
struct __attribute__((packed)) dbf_hdr
{
    uint32_t    blks;
};

/* variable length data */
struct __attribute__((packed)) dbf_va
{
    int16_t off;
    int16_t len;
};


/* Database file record slot */
struct __attribute__((packed)) dbf_rec
{
    uint16_t    off;
    int16_t     sz;
};

/* Database file data block header */
struct __attribute__((packed)) dbf_blkhdr
{
    uint16_t        nrec;
    uint16_t        free;
    struct dbf_rec  rec[0]; 
};

struct dbf
{
    int             fd;
    struct dbf_hdr  *hdr;
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
void f_crt(struct dbf *f, char filename[]);
void f_open(struct dbf *f, char filename[]);
void f_close(struct dbf *f);
void f_binit(char bd[BLK_SZ]);
void f_nr(struct dbf *f, char *r, int size);
void f_dr(struct dbf_it *it);
void f_ur(struct dbf_it *it, char *r, int size);

char *b_nr(char *block, int size);

/* file iterator */
void f_it(struct dbf *f, struct dbf_it *it);
char *f_itnext(struct dbf_it *it);
void f_itfree(struct dbf_it *it);

void f_strcpy(char *s, struct dbf_va *v, char *r);

#endif

