#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exitcode.h"
#include "data.h"

int64_t d_integer(char *s, int l)
{
    switch (l)
    {
        case 1:
            return *s;
        case 2:
            return *((int16_t *)s);
        case 4:
            return *((int32_t *)s);
        case 8:
            return *((int64_t *)s);
        default:
            fprintf(stderr, "invalid integer");
            exit(EC_DB);
    }
}

double d_float(char *s, int l)
{
    switch (l)
    {
        case 4:
            return *((float *)s);
        case 8:
            return *((double *)s);
        default:
            fprintf(stderr, "invalid float");
            exit(EC_DB);
    }
}

void d_varchar(char *s, char *f, int l)
{
    strncpy(s, f, l);
    s[l] = 0;
}

struct d_datum_h *d_btoh(struct d_datum_b *b)
{
    struct d_datum_h *h;
    char *p;

    if ( (h = malloc(sizeof(struct d_datum_h))) == 0)
    {
        perror("d_btoh malloc");
        exit(EC_M);
    }

    h->domain = b->domain;

    switch (b->domain)
    {
        case DOMAIN_VARCHAR:
            if ( (p = malloc(b->len)) == 0)
            {
                perror("d_bto_h malloc");
                exit(EC_M);
            }
            d_varchar(p, b->bytes, b->len);  
            h->v.v_val = p;
            break;
        case DOMAIN_INTEGER:
            h->v.i_val = d_integer(b->bytes, b->len);
            break;
        case DOMAIN_FLOAT:
            h->v.f_val = d_float(b->bytes, b->len);
            break;
    }

    return h;
}

void d_hfree(struct d_datum_h *h)
{
    if (h->domain == DOMAIN_VARCHAR)
    {
        free(h->v.v_val);
    }
    free(h);
}

char *d_text(char *val, struct d_datum_h *v)
{
    switch (v->domain)
    {
        case DOMAIN_VARCHAR:
            strcpy(val, v->v.v_val);
            break;
        case DOMAIN_INTEGER:
            sprintf(val, 
                    "%lld",
                    v->v.i_val);
            break;
        case DOMAIN_FLOAT:
            sprintf(val,
                    "%lf",
                    v->v.f_val);
            break;
    }

    return val;
}
