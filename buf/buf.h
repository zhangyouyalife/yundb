#ifndef _BUF_
#define _BUF_

#include "conf.h"
#include "file.h"

#define BUF_SZ  100

#define BF_DIRTY    1

struct buf
{
    struct dbf *f;
    int b;
    int ref;
    int flag;
} buffer[BUF_SZ];

char blocks[BLK_SZ * BUF_SZ];

#define B_BUF(b)        (&buffer[((b) - blocks)/BLK_SZ])
#define B_BLK(b)        (blocks + \
                        BLK_SZ * ((b) - buffer)/sizeof(struct buf))
#define IS_DIRTY(b)     (b->flag & BF_DIRTY)
#define SET_DIRTY(b)    (b->flag |= BF_DIRTY)
#define CLR_DIRTY(b)    (b->flag &= (~BF_DIRTY))

char *b_get(struct dbf *f, int b);
void b_put(char *b);
void b_pin(char *b);
void b_unp(char *b);
void b_fw(char *b);

#endif
