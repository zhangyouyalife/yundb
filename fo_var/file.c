#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "file.h"
#include "block.h"

void FileCreate(char filename[])
{
    int fd;
    struct FileHeader *h;

    if ((fd = open(filename, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0)
    {
        perror("FileCreate open failed");
        exit(EC_IO);
    }

    h = (struct FileHeader *) block0;
    h->fh_blocks = 1;
    
    BlockWrite(fd, 0, block0);

    if ( -1 == close(fd))
    {
        perror("FileCreate close failed");
        exit(EC_IO);
    }
}

void FileOpen(char filename[])
{
    if ((file_fd = open(filename, O_RDWR)) < 0)
    {
        perror("FileOpen open failed");
        exit(EC_IO);
    }

    BlockRead(file_fd, 0, block0);

    file_header = (struct FileHeader *) block0;
}

void FileClose()
{
    BlockWrite(file_fd, 0, block0);

    if ( -1 == close(file_fd))
    {
        perror("FileClose close failed");
        exit(EC_IO);
    }
}

