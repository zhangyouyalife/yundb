#ifndef _DATA_
#define _DATA_

#include <stdint.h>

struct d_datum_b
{
    uint8_t domain;
    char *bytes;
    uint16_t len;
};

struct d_datum_h
{
    uint8_t domain;
    union
    {
        int64_t i_val;
        double f_val;
        char *v_val;
    } v;
};

#define DOMAIN_INTEGER  1
#define DOMAIN_FLOAT    2
#define DOMAIN_VARCHAR  3

struct d_datum_h *d_btoh(struct d_datum_b *b);
void d_hfree(struct d_datum_h *h);

#endif

