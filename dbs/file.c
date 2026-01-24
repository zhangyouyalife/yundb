#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file.h"
#include "block.h"

void f_bs(struct dbf *f, int bn)
{
    if (-1 == lseek(f->fd, bn * BLK_SZ, SEEK_SET))
    {
        perror("f_bs lseek error");
        exit(EC_IO);
    }
}

void f_wb(struct dbf *f, int bn, char bd[BLK_SZ])
{
    f_bs(f, bn);

    if (BLK_SZ != write(f->fd, bd, BLK_SZ))
    {
        perror("f_wb write error");
        exit(EC_IO);
    }
}

void f_rb(struct dbf *f, int bn, char bd[BLK_SZ])
{
    int r;
    char *p;

    f_bs(f, bn);

    p = bd;
    while ((r = read(f->fd, p, bd + BLK_SZ - p)) > 0)
        p += r;

    if (p - bd != BLK_SZ)
    {
        perror("f_rb read error");
        exit(EC_IO);
    }
}

void f_crt(struct dbf *f, char filename[])
{
    int fd;

    if ((fd = open(filename, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0)
    {
        perror("f_crt open failed");
        exit(EC_IO);
    }

    f->fd = fd;
    f->hdr = (struct dbf_hdr *) f->blk0;
    f->hdr->blks = 1;
    
    f_wb(f, 0, f->blk0);

    if ( -1 == close(f->fd))
    {
        perror("f_crt close failed");
        exit(EC_IO);
    }
}

void f_open(struct dbf *f, char filename[])
{
    int fd;

    if ((fd = open(filename, O_RDWR)) < 0)
    {
        perror("f_open open failed");
        exit(EC_IO);
    }

    f->fd = fd;
    f->hdr = (struct dbf_hdr *) f->blk0;

    f_rb(f, 0, f->blk0);
}

void f_close(struct dbf *f)
{
    f_wb(f, 0, f->blk0);

    if ( -1 == close(f->fd))
    {
        perror("f_close close failed");
        exit(EC_IO);
    }
}

void f_binit(char bd[BLK_SZ])
{
    struct dbf_blkhdr *h;
    
    h = (struct dbf_blkhdr *) bd;
    h->nrec = 0;
    h->free = BLK_SZ;
}

char *f_nr(char *block, int size)
{
    int free;
    struct dbf_blkhdr *bh;

    bh = (struct dbf_blkhdr *) block;

    free = bh->free 
        - sizeof(struct dbf_blkhdr)
        - sizeof(struct dbf_rec) * bh->nrec;

    if (free < size)
        return NULL;

    bh->rec[bh->nrec].off = bh->free - size;
    bh->rec[bh->nrec].sz = size;
    bh->free -= size;
    bh->nrec++;

    return (block + bh->free);
}

