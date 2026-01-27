#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exitcode.h"
#include "db.h"
#include "file.h"
#include "datadict.h"
#include "data.h"

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
    union db_value v;

    db_attr_val(&v, pos, r, d, val);

    a = d->attrs + pos;
    switch (a->domain)
    {
        case DOMAIN_VARCHAR:
            break;
        case DOMAIN_INTEGER:
            sprintf(val, 
                    "%lld",
                    v.i_val);
            break;
        case DOMAIN_FLOAT:
            sprintf(val,
                    "%lf",
                    v.f_val);
            break;
    }
}


int db_attr_off(int pos, struct dd_reldesc *d)
{
    int off; 
    int i;
    struct dd_attrdesc *a;

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

    return off;
}

void db_attr_val(union db_value *v, int pos, 
        char *r, struct dd_reldesc *d,
        char *buf)
{
    int off; 
    struct dd_attrdesc *a;
    struct dbf_va *va;

    off = db_attr_off(pos, d);

    a = d->attrs + pos;
    switch (a->domain)
    {
        case DOMAIN_VARCHAR:
            va = (struct dbf_va *) (r + off);
            db_varchar(buf, r + va->off, va->len);  
            v->v_val = buf;
            break;
        case DOMAIN_INTEGER:
            v->i_val = db_integer(r + off, a->len);
            break;
        case DOMAIN_FLOAT:
            v->f_val = db_float(r + off, a->len);
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
    f_nr(&dd_get("relation")->f, r, DD_REL_RSZ(c->name));
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
        f_nr(&dd_get("attribute")->f, r, DD_ATTR_RSZ(c->name, a->name));
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

void dml_r(struct dml_rec *rec, 
        union db_value *values, struct dd_reldesc *d)
{
    int size, vaoff, len;
    int i;
    struct dd_attrdesc *a;
    union db_value *v;
    char *r, *p;

    /* calculate size */
    size = 0;
    vaoff = 0;
    for (i = 0; i < d->nattr; i++)
    {
        a = &d->attrs[i];
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
    for (i = 0; i < d->nattr; i++)
    {
        a = &d->attrs[i];
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

    rec->sz = size;
    rec->r = r;
}

void dml_rfree(struct dml_rec *r)
{
    free(r->r);
}

int dml_insert(char *rname, union db_value *values)
{
    struct dml_rec r;
    struct dd_rel_m *rel;

    if ( !(rel = dd_get(rname)) )
    {
        return E_REL_NOT_FOUND;
    }

    dml_r(&r, values, &rel->desc);

    f_nr(&rel->f, r.r, r.sz);

    dml_rfree(&r);

    return 0;
}

int dml_delete(char *rname, struct db_where *w)
{
    struct dd_rel_m *rel;
    struct dd_attrdesc *a, *at;
    union db_value v;
    struct dbf_it it;
    struct dbf f;
    int size, vaoff, len, i;
    char *r, *p;
    char fn[256], *buf;

    if ( !(rel = dd_get(rname)) )
    {
        return E_REL_NOT_FOUND;
    }

    at = 0;
    for (i = 0; i < rel->desc.nattr; i++)
    {
        a = &rel->desc.attrs[i];
        if (strcmp(a->name, w->attr) == 0)
        {
            at = a;
            break;
        }
    }
    if (at == 0)
    {
        return E_ATTR_NOT_FOUND;
    }


    f_it(&rel->f, &it);
    while ( (r = f_itnext(&it)) != 0)
    {
        switch (at->domain)
        {
            case DOMAIN_INTEGER:
                db_attr_val(&v, at->pos, r, &rel->desc, buf);
                if (w->v.i_val == v.i_val) 
                {
                    f_dr(&it);
                }
                break;
            case DOMAIN_FLOAT:
                db_attr_val(&v, at->pos, r, &rel->desc, buf);
                if (w->v.f_val == v.f_val) 
                {
                    f_dr(&it);
                }
                break;
            case DOMAIN_VARCHAR:
                if ( (buf = malloc(at->len + 1)) == 0)
                {
                    perror("dml_delete malloc buf");
                    exit(EC_M);
                }
                db_attr_val(&v, at->pos, r, &rel->desc, buf);
                if (strcmp(w->v.v_val, v.v_val) == 0) 
                {
                    f_dr(&it);
                }
                free(buf);
                break;
        }
    }

    f_itfree(&it);

    return 0;
}

int dml_update(char *rname, union db_value *values, struct db_where *w)
{
    struct dd_rel_m *rel;
    struct dd_attrdesc *a, *at;
    union db_value v;
    struct dbf_it it;
    struct dbf f;
    int size, vaoff, len, i;
    char *r, *nr, *p;
    char fn[256], *buf;
    struct dml_rec rec;

    if ( !(rel=dd_get(rname)))
    {
        return E_REL_NOT_FOUND;
    }

    if ( (at = dd_reldesc_attr(w->attr, &rel->desc)) == 0)
    {
        return E_ATTR_NOT_FOUND;
    }

    f_it(&rel->f, &it);
    while ( (r = f_itnext(&it)) != 0)
    {
        switch (at->domain)
        {
            case DOMAIN_INTEGER:
                db_attr_val(&v, at->pos, r, &rel->desc, buf);
                if (w->v.i_val == v.i_val) 
                {
                    dml_r(&rec, values, &rel->desc);
                    f_ur(&it, rec.r, rec.sz);
                    dml_rfree(&rec);
                }
                break;
            case DOMAIN_FLOAT:
                db_attr_val(&v, at->pos, r, &rel->desc, buf);
                if (w->v.f_val == v.f_val) 
                {
                    dml_r(&rec, values, &rel->desc);
                    f_ur(&it, rec.r, rec.sz);
                    dml_rfree(&rec);
                }
                break;
            case DOMAIN_VARCHAR:
                if ( (buf = malloc(at->len + 1)) == 0)
                {
                    perror("dml_delete malloc buf");
                    exit(EC_M);
                }
                db_attr_val(&v, at->pos, r, &rel->desc, buf);
                if (strcmp(w->v.v_val, v.v_val) == 0) 
                {
                    dml_r(&rec, values, &rel->desc);
                    f_ur(&it, rec.r, rec.sz);
                    dml_rfree(&rec);
                }
                free(buf);
                break;
        }
    }

    f_itfree(&it);

    return 0;
}
