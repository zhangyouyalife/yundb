#ifndef _FILE_
#define _FILE_

#include "block.h"

struct FileHeader
{
    uint32_t fh_blocks;
};

int file_fd;
struct FileHeader *file_header;
char block0[BLOCK_SZ];

void FileCreate(char filename[]);
void FileOpen(char filename[]);
void FileClose();

#endif

