/* sysfile.c: os file */
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "sysfile.h"

int sf_blks(int fd)
{
    struct stat st;
    if (fstat(fd, &st) == 0)
    {
        return st.st_size / BLK_SZ;
    }
    else
    {
        perror("sf_blks fstat error");
        exit(EC_IO);
    }
}

static void sf_bs(int fd, int bn)
{
    if (-1 == lseek(fd, bn * BLK_SZ, SEEK_SET))
    {
        perror("f_bs lseek error");
        exit(EC_IO);
    }
}

void sf_wb(int fd, int bn, char bd[BLK_SZ])
{
    sf_bs(fd, bn);

    if (BLK_SZ != write(fd, bd, BLK_SZ))
    {
        perror("sf_wb write error");
        exit(EC_IO);
    }
}

void sf_rb(int fd, int bn, char bd[BLK_SZ])
{
    int r;
    char *p;

    sf_bs(fd, bn);

    p = bd;
    while ((r = read(fd, p, bd + BLK_SZ - p)) > 0)
        p += r;

    if (p - bd != BLK_SZ)
    {
        perror("sf_rb read error");
        exit(EC_IO);
    }
}
