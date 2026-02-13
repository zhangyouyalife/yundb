#include <string.h>

#include "tuple.h"
#include "data.h"

void t_varchar(char *s, struct t_va *v, char *t)
{
    struct d_datum_b b;
    struct d_datum_h *h;

    b.domain = DOMAIN_VARCHAR;
    b.bytes = t + v->off;
    b.len = v->len;

    h = d_btoh(&b);

    strncpy(s, h->v.v_val, v->len);
    s[v->len] = 0;

    d_hfree(h);
}
