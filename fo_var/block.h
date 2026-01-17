#ifndef _BLOCK_
#define _BLOCK_

#include "record.h"

#define BLOCK_SZ    256

char block[BLOCK_SZ];

struct __attribute__((packed)) BlockRecord {
    uint16_t    br_off;
    int16_t     size;
};

struct __attribute__((packed))  BlockHeader {
    uint16_t    bh_nrec;
    uint16_t    bh_free_end;
    struct BlockRecord bh_rec[0];
};

void BlockRead(int fd, int bno, char *bd);
void BlockWrite(int fd, int bno, char *bd);
void BlockNew(char *bd);
struct Record *BlockNewRecord(char *block, int size);

#endif

