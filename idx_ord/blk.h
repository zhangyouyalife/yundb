#ifndef _BLK_
#define _BLK_

#include <stdint.h>

/* block record slot */
struct __attribute__((packed)) blk_tuple
{
    uint16_t    off;
    int16_t     sz;
};

/* block header */
struct __attribute__((packed)) blk_hdr
{
    uint16_t            ntuple;
    uint16_t            free;
    struct blk_tuple    tuples[0]; 
};

void blk_init(char *blk);
int blk_nt(char *blk, int size);
void blk_dt(char *blk, int tn);
void blk_ut(char *blk, int tn, char *t, int sz);

int blk_freespace(char *blk);

#define BLK_GET(blk, tn)    (&((struct blk_hdr *) (blk))->tuples[tn])
#define BLK_ISNONE(t)      ((t)->sz == -1)
#define BLK_SETNONE(t)     ((t)->sz = -1)

#endif

