#ifndef _FILE_
#define _FILE_

#include "block.h"

/* Database file header */ 
struct __attribute__((packed)) dbf_hdr
{
    uint32_t    blks;
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

void f_bs(struct dbf *f, int bn);
void f_wb(struct dbf *f, int bn, char bd[BLK_SZ]);
void f_rb(struct dbf *f, int bn, char bd[BLK_SZ]);
void f_crt(struct dbf *f, char filename[]);
void f_open(struct dbf *f, char filename[]);
void f_close(struct dbf *f);
void f_binit(char bd[BLK_SZ]);
char *f_nr(char *block, int size);

#endif

