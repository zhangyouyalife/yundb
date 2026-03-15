/* createdb.c */
#include <stdlib.h>
#include <stdio.h>

#include "datadict.h"
#include "block/block.h"
#include "buf.h"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        puts("Usage:");
        puts("\tcreatedb <db_path>");
        exit(1);
    }
    
    b_init();

    blk_types_init();

    dd_create(argv[1]);

    b_sync();

    exit(0);
}
