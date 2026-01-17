#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "block.h"

void BlockSeek(int fd, int bno)
{
    if (-1 == lseek(fd, bno * BLOCK_SZ, SEEK_SET))
    {
        perror("BlockSeek lseek error");
        exit(EC_IO);
    }
}

void BlockWrite(int fd, int bno, char *bd)
{
    BlockSeek(fd, bno);

    if (BLOCK_SZ != write(fd, bd, BLOCK_SZ))
    {
        perror("BlockWrite write error");
        exit(EC_IO);
    }
}

void BlockRead(int fd, int bno, char *bd)
{
    int r;
    char *p;

    BlockSeek(fd, bno);

    p = bd;
    while ((r = read(fd, p, bd + BLOCK_SZ - p)) > 0)
        p += r;

    if (p - bd != BLOCK_SZ)
    {
        perror("BlockRead read error");
        exit(EC_IO);
    }
}

void BlockNew(char *bd)
{
    struct BlockHeader *h;

    h = (struct BlockHeader*) bd;
    
    h->bh_nrec = 0;
    h->bh_free_end = BLOCK_SZ;
}

struct Record *BlockNewRecord(char *block, int size)
{
    int free;
    struct BlockHeader *bh;

    bh = (struct BlockHeader*) block;

    free = bh->bh_free_end 
        - sizeof(struct BlockHeader)
        - sizeof(struct BlockRecord) * bh->bh_nrec;

    if (free < size)
        return NULL;

    bh->bh_rec[bh->bh_nrec].br_off = bh->bh_free_end - size;
    bh->bh_rec[bh->bh_nrec].size = size;
    bh->bh_free_end -= size;
    bh->bh_nrec++;

    return (struct Record *) (block + bh->bh_free_end);
}
