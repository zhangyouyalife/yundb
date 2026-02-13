/* createdb.c */
#include <stdlib.h>
#include <stdio.h>

#include "datadict.h"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        puts("Usage:");
        puts("\tcreatedb <db_path>");
        exit(1);
    }

    dd_create(argv[1]);
    exit(0);
}
