#ifndef _BUF_
#define _BUF_

#include "conf.h"
#include "sysfile.h"

#define BUF_SZ  20

#define BF_DIRTY    1

struct buf
{
    int fd;
    int b;
    int ref;
    int flag;
} buffer[BUF_SZ];

char blocks[BLK_SZ * BUF_SZ];

#define B_BUF(b)        (&buffer[((b) - blocks)/BLK_SZ])
#define B_BLK(b)        (blocks + \
                        BLK_SZ * ((b) - buffer))
#define IS_DIRTY(b)     (b->flag & BF_DIRTY)
#define SET_DIRTY(b)    (b->flag |= BF_DIRTY)
#define CLR_DIRTY(b)    (b->flag &= (~BF_DIRTY))

#define B_EMPTY(b)      ((b)->fd == -1)
#define B_SET_EMPTY(b)  ((b)->fd = -1)

char *b_get(int fd, int b);
void b_put(char *b);
void b_pin(char *b);
void b_unp(char *b);
void b_fw(char *b);

void b_init();
void b_sync();
void b_info();

#endif
