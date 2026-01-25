#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exitcode.h"
#include "db.h"
#include "file.h"
#include "datadict.h"

void db_crt_relation();

void db_crt_attribute();

void db_init(char *path)
{
    static char dblk[BLK_SZ];
    char *r;
    char tpath[256];

    if ( -1 == mkdir(path, 0755) )
    {
        perror("db_init mkdir failed");
        exit(EC_IO);
    }

    /* create relation.rel */
    sprintf(tpath, "%s%s.rel", path, REL_NAME);
    f_crt(&relation, tpath);

    /* create attribute */
    sprintf(tpath, "%s%s.rel", path, ATTR_NAME);
    f_crt(&attribute, tpath);

    db_crt_relation();

    db_crt_attribute();

    f_close(&attribute);
    f_close(&relation);

}

void db_crt_attribute()
{
    char *r;

    /* relation record */
    if ( (r = malloc(DD_REL_RSZ(ATTR_NAME))) == 0)
    {
        perror("db_crt_attribute malloc failed");
        exit(EC_M);
    }
    dd_rel(ATTR_NAME, 5, FO_HEAP, r); 
    f_nr(&relation, r, DD_REL_RSZ(ATTR_NAME));
    free(r);

    /* attribute records */
    /* attr: attribute.rel */
    if ( (r = malloc(DD_ATTR_RSZ(ATTR_NAME, "rel"))) == 0)
    {
        perror("db_crt_attribute malloc failed");
        exit(EC_M);
    }
    dd_attr(ATTR_NAME, "rel", DOMAIN_VARCHAR, 0, 256, r);
    f_nr(&attribute, r, DD_ATTR_RSZ(ATTR_NAME, "rel"));
    free(r);
    /* attr: attribute.name */
    if ( (r = malloc(DD_ATTR_RSZ(ATTR_NAME, "name"))) == 0)
    {
        perror("db_crt_attribute malloc failed");
        exit(EC_M);
    }
    dd_attr(ATTR_NAME, "name", DOMAIN_VARCHAR, 1, 256, r);
    f_nr(&attribute, r, DD_ATTR_RSZ(ATTR_NAME, "name"));
    free(r);

    /* attr: relation.domain */
    if ( (r = malloc(DD_ATTR_RSZ(ATTR_NAME, "domain"))) == 0)
    {
        perror("db_crt_attribute malloc failed");
        exit(EC_M);
    }
    dd_attr(ATTR_NAME, "domain", DOMAIN_INTEGER, 2, 1, r);
    f_nr(&attribute, r, DD_ATTR_RSZ(ATTR_NAME, "domain"));
    free(r);

    /* attr: relation.pos */
    if ( (r = malloc(DD_ATTR_RSZ(ATTR_NAME, "pos"))) == 0)
    {
        perror("db_crt_attribute malloc failed");
        exit(EC_M);
    }
    dd_attr(ATTR_NAME, "pos", DOMAIN_INTEGER, 3, 2, r);
    f_nr(&attribute, r, DD_ATTR_RSZ(ATTR_NAME, "pos"));
    free(r);

    /* attr: relation.pos */
    if ( (r = malloc(DD_ATTR_RSZ(ATTR_NAME, "len"))) == 0)
    {
        perror("db_crt_attribute malloc failed");
        exit(EC_M);
    }
    dd_attr(ATTR_NAME, "len", DOMAIN_INTEGER, 4, 2, r);
    f_nr(&attribute, r, DD_ATTR_RSZ(ATTR_NAME, "len"));
    free(r);
}

void db_crt_relation()
{
    char *r;

    /* relation record */
    if ( (r = malloc(DD_REL_RSZ(REL_NAME))) == 0)
    {
        perror("db_crt_relation malloc failed");
        exit(EC_M);
    }
    dd_rel(REL_NAME, 3, FO_HEAP, r); 
    f_nr(&relation, r, DD_REL_RSZ(REL_NAME));
    free(r);

    /* attribute records */
    /* attr: relation.name */
    if ( (r = malloc(DD_ATTR_RSZ(REL_NAME, "name"))) == 0)
    {
        perror("db_crt_relation malloc failed");
        exit(EC_M);
    }
    dd_attr(REL_NAME, "name", DOMAIN_VARCHAR, 0, 256, r);
    f_nr(&attribute, r, DD_ATTR_RSZ(REL_NAME, "name"));
    free(r);

    /* attr: relation.nattr */
    if ( (r = malloc(DD_ATTR_RSZ(REL_NAME, "nattr"))) == 0)
    {
        perror("db_crt_relation malloc failed");
        exit(EC_M);
    }
    dd_attr(REL_NAME, "nattr", DOMAIN_INTEGER, 1, 2, r);
    f_nr(&attribute, r, DD_ATTR_RSZ(REL_NAME, "nattr"));
    free(r);

    /* attr: relation.forg */
    if ( (r = malloc(DD_ATTR_RSZ(REL_NAME, "forg"))) == 0)
    {
        perror("db_crt_relation malloc failed");
        exit(EC_M);
    }
    dd_attr(REL_NAME, "forg", DOMAIN_INTEGER, 2, 1, r);
    f_nr(&attribute, r, DD_ATTR_RSZ(REL_NAME, "forg"));
    free(r);
}

int64_t db_integer(char *s, int l)
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

double db_float(char *s, int l)
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

void db_varchar(char *s, char *f, int l)
{
    strncpy(s, f, l);
    s[l] = 0;
}

void db_val(char *val, int pos, char *r, struct dd_reldesc *d)
{
    int off, len; 
    int i;
    struct dd_attrdesc *a;
    struct dbf_va *va;


    a = d->attrs;
    for (i = 0, off = 0; i < pos; i++, a++) {
        switch (a->domain)
        {
            case DOMAIN_INTEGER:
            case DOMAIN_FLOAT:
                off += a->len;
                break;
           case DOMAIN_VARCHAR:
                off += 4;
        }
    }

    a = d->attrs + pos;
    switch (a->domain)
    {
        case DOMAIN_VARCHAR:
            va = (struct dbf_va *) (r + off);
            db_varchar(val, r + va->off, va->len);  
            break;
        case DOMAIN_INTEGER:
            sprintf(val, 
                    "%lld",
                    db_integer(r + off, a->len));
            break;
        case DOMAIN_FLOAT:
            sprintf(val,
                    "%lf",
                    db_float(r + off, a->len));
            break;
    }
}


void ddl_create_new(struct ddl_create *c, 
        char *name, uint16_t nattr, uint8_t forg)
{
    char *s;

    if ( (s = malloc(strlen(name) + 1)) == 0)
    {
        perror("ddl_create_new malloc failed");
        exit(EC_M);
    }

    strcpy(s, name);
    c->name = s;
    c->nattr = nattr;
    c->forg = forg;

    if ( (s = calloc(nattr, sizeof(struct ddl_attr))) == 0)
    {
        perror("ddl_create_new calloc for attrs failed");
        exit(EC_M);
    }
    c->attrs = (struct ddl_attr *) s;
}

void ddl_create_attr(struct ddl_create *c, char *name, uint16_t pos, 
        uint8_t domain, uint16_t len)
{
    char *s;
    struct ddl_attr *a;

    if ( (s = malloc(strlen(name) + 1)) == 0)
    {
        perror("ddl_create_new malloc failed");
        exit(EC_M);
    }
    strcpy(s, name);
    a = c->attrs + pos;

    a->name = s;
    a->domain = domain;
    a->pos = pos;
    a->len = len;
}

void ddl_create_free(struct ddl_create *c)
{
    struct ddl_attr *a, *e;

    free(c->name);

    e = c->attrs + c->nattr;
    for (a = c->attrs ; a < e ; a++)
    {
        free(a->name);
    }

    free(c->attrs);
}

void ddl_create_go(struct ddl_create *c)
{
    char *r;
    int i;
    struct ddl_attr *a;
    char tpath[256];
    struct dbf f;

    /* relation record */
    if ( (r = malloc(DD_REL_RSZ(c->name))) == 0)
    {
        perror("db_create_go malloc failed");
        exit(EC_M);
    }
    dd_rel(c->name, c->nattr, c->forg, r); 
    f_nr(&relation, r, DD_REL_RSZ(c->name));
    free(r);

    /* attribute records */
    for (i = 0, a = c->attrs; i < c->nattr; i++, a++)
    {
        if ( (r = malloc(DD_ATTR_RSZ(c->name, a->name))) == 0)
        {
            perror("db_create_go malloc failed");
            exit(EC_M);
        }
        dd_attr(c->name, a->name, a->domain, a->pos, a->len, r);
        f_nr(&attribute, r, DD_ATTR_RSZ(c->name, a->name));
        free(r);
    }

    /* create file */
    sprintf(tpath, "%s%s.rel", db_path, c->name);
    f_crt(&f, tpath);
    f_close(&f);
}

void ddl_drop(char *name)
{
    
}

int dml_insert(char *rel, union db_value *values)
{
    struct dd_reldesc d;
    struct dd_attrdesc *a;
    union db_value *v;
    struct dbf_it it;
    struct dbf f;
    int size, vaoff, len, i;
    char *r, *p;
    char fn[256];

    if (!dd_reldesc_get(&d, rel))
    {
        return E_REL_NOT_FOUND;
    }

    sprintf(fn, "%s%s.rel", db_path, rel);
    f_open(&f, fn);
    /* calculate size */
    size = 0;
    vaoff = 0;
    for (i = 0; i < d.nattr; i++)
    {
        a = &d.attrs[i];
        v = &values[i];
        switch(a->domain)
        {
            case DOMAIN_INTEGER:
            case DOMAIN_FLOAT:
                size += a->len;
                vaoff += a->len;
                break;
            case DOMAIN_VARCHAR:
                size += sizeof(struct dbf_va) + strlen(v->v_val);
                vaoff += sizeof(struct dbf_va);
        }
    }
    /* construct record */
    if ( (r = malloc(size)) == 0)
    {
        perror("dml_insert malloc for temp record");
        exit(EC_M);
    }
    p = r;
    for (i = 0; i < d.nattr; i++)
    {
        a = &d.attrs[i];
        v = &values[i];
        switch(a->domain)
        {
            case DOMAIN_INTEGER:
                memcpy(p, &v->i_val, a->len);
                p += a->len;
                break;
            case DOMAIN_FLOAT:
                if (a->len == 4)
                    *(float *)p = (float) v->f_val;
                else
                    *(double *)p = v->f_val;
                p += a->len;
                break;
            case DOMAIN_VARCHAR:
                ((struct dbf_va *)p)->off = vaoff;
                len = strlen(v->v_val);
                ((struct dbf_va *)p)->len = len;
                memcpy(r + vaoff, v->v_val, len);
                vaoff += len;
                p += sizeof(struct dbf_va);
        }
    }

    f_nr(&f, r, size);
    free(r);

    f_close(&f);

    dd_reldesc_free(&d);

    return 0;
}

