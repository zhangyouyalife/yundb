#ifndef _TUPLE_
#define _TUPLE_

#include <unistd.h>

/* variable length data */
struct __attribute__((packed)) t_va
{
    int16_t off;
    int16_t len;
};

void t_varchar(char *s, struct t_va *v, char *t);

#endif

