#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exitcode.h"
#include "db.h"
#include "file.h"
#include "datadict.h"
#include "data.h"
#include "buf.h"
#include "block/block.h"

struct d_datum_h *db_attr_val(int pos, char *r, struct dd_reldesc *d)
{
    int off; 
    struct dd_attrdesc *a;
    struct t_va *va;
    struct d_datum_b b;

    off = dd_attr_off(pos, d);

    a = d->attrs + pos;

    b.domain = a->domain;
    if (a->domain == DOMAIN_VARCHAR)
    {
        va = (struct t_va *) (r + off);
        b.bytes = r + va->off;
        b.len = va->len;
    } else {
        b.bytes = r + off;
        b.len = a->len;
    }

    return d_btoh(&b);
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
    int i;
    struct ddl_attr *a;
    char tpath[256];
    struct dbf f;
    union dml_value v[5];

    /* relation record */
    if (0 != dd_get(c->name))
    {
        printf("%s already exists", c->name);
        return;
    }

    v[0].v_val = c->name;
    v[1].i_val = c->nattr;
    v[2].i_val = c->forg;
    dml_insert("relation", v);

    /* attribute records */
    for (i = 0, a = c->attrs; i < c->nattr; i++, a++)
    {
        v[0].v_val = c->name;
        v[1].v_val = a->name;
        v[2].i_val = a->domain;
        v[3].i_val = a->pos;
        v[4].i_val = a->len;
        dml_insert("attribute", v);
    }

    /* create file */
    sprintf(tpath, "%s/%s.rel", db_path, c->name);
    f_crt(&f, tpath, c->forg);
    b_clearfd(f.fd);
    f_close(&f);

    /* add to datadict in memory */
    /* TODO */
    b_sync();
    dd_sync();

    dd_add(c->name);
}

void ddl_drop(char *name)
{
    
}

/* create tuple */
void dml_r(struct dml_rec *rec, 
        union dml_value *values, struct dd_reldesc *d)
{
    int size, vaoff, len;
    int i;
    struct dd_attrdesc *a;
    union dml_value *v;
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
                size += sizeof(struct t_va) + strlen(v->v_val);
                vaoff += sizeof(struct t_va);
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
                ((struct t_va *)p)->off = vaoff;
                len = strlen(v->v_val);
                ((struct t_va *)p)->len = len;
                memcpy(r + vaoff, v->v_val, len);
                vaoff += len;
                p += sizeof(struct t_va);
        }
    }

    rec->sz = size;
    rec->r = r;
}

void dml_rfree(struct dml_rec *r)
{
    free(r->r);
}

void
db_dr(struct dql_cursor *cur)
{
    char *blk;

    blk = b_get(cur->r->f.fd, cur->it.b);

    blk_entry_delete(blk, cur->it.r);
   
    SET_DIRTY(B_BUF(blk)); 
    b_put(blk);
}

static int cmp_ins(char *t1, char *t2)
{
    struct t_va *v1, *v2;
    char *d1, *d2;
    int i, res;

    v1 = (struct t_va *)t1;
    v2 = (struct t_va *)t2;

    d1 = t1 + v1->off;
    d2 = t2 + v1->off;

    for (i = 0; i < v1->len && i < v2->len && d1[i] == d2[i]; i++) ;

    if (i < v1->len && i < v2->len)
    {
        res = d1[i] - d2[i];
    }
    else
    {
        res = v1->len - v2->len;
    }

    return res;
}

int dml_insert(char *rname, union dml_value *values)
{
    struct dml_rec r;
    struct dd_rel_m *rel;

    if ( !(rel = dd_get(rname)) )
    {
        return E_REL_NOT_FOUND;
    }

    dml_r(&r, values, &rel->desc);

    if (strcmp("instructor", rname))
    {
        f_nr(&rel->f, r.r, r.sz, 0);
    }
    else
    {
        f_nr(&rel->f, r.r, r.sz, cmp_ins);
    }

    dml_rfree(&r);

    return 0;
}

int dml_delete(char *rname, struct dml_where *w)
{
    struct dd_rel_m *rel;
    struct dd_attrdesc *at;
    int i;
    char val[256];
    struct d_datum_h *v;
    struct dql_cursor cur;
    struct dql_tuple t;

    if ( !(rel = dd_get(rname)) )
    {
        return E_REL_NOT_FOUND;
    }

    if ( (at = dd_reldesc_attr(w->attr, &rel->desc)) == 0)
    {
        return E_ATTR_NOT_FOUND;
    }

    dql_cursor_create(&cur, rname);
    if (0 != dql_cursor_open(&cur) )
    {
        perror("list_records dql_cursor_open");
        return E_REL_NOT_FOUND;
    }

    while ( dql_cursor_fetch(&t, &cur)  == 0)
    {
        v = db_attr_val(at->pos, t.t, &rel->desc);
        
            switch (v->domain)
            {
                case DOMAIN_INTEGER:
                    if (w->v.i_val == v->v.i_val) 
                    {
                        db_dr(&cur);
                    }
                    break;
                case DOMAIN_FLOAT:
                    if (w->v.f_val == v->v.f_val) 
                    {
                        db_dr(&cur);
                    }
                    break;
                case DOMAIN_VARCHAR:
                    if (strcmp(w->v.v_val, v->v.v_val) == 0) 
                    {
                        db_dr(&cur);
                    }
                    break;
            }

        d_hfree(v);
    }
    dql_cursor_close(&cur);

    return 0;
}


void dml_update_cur(struct dql_cursor *cur, union dml_value *values)
{
    struct dml_rec rec;
    char *blk;

    dml_r(&rec, values, &cur->r->desc);

    blk = b_get(cur->r->f.fd, cur->it.b);

    if (blk_entry_update(blk, cur->it.r, rec.sz) == 0)
    {
        /* update in block */
        memcpy(blk_record(blk, cur->it.r), rec.r, rec.sz);

        SET_DIRTY(B_BUF(blk));
        b_put(blk);
    }
    else
    {
        /* delete and insert */
        db_dr(cur);
        f_nr(&cur->r->f, rec.r, rec.sz, 0);
    }

    dml_rfree(&rec);
}

int dml_update(char *rname, union dml_value *values, struct dml_where *w)
{
    struct dd_rel_m *rel;
    struct dd_attrdesc *a, *at;
    struct d_datum_h *v;
    struct dql_cursor cur;
    struct dql_tuple t;

    if ( !(rel=dd_get(rname)))
    {
        return E_REL_NOT_FOUND;
    }

    if ( (at = dd_reldesc_attr(w->attr, &rel->desc)) == 0)
    {
        return E_ATTR_NOT_FOUND;
    }

    dql_cursor_create(&cur, rname);
    if ( 0 != dql_cursor_open(&cur))
    {
        return E_REL_NOT_FOUND;
    }

    while ( dql_cursor_fetch(&t, &cur) == 0 )
    {
        v = db_attr_val(at->pos, t.t, &rel->desc);

        switch (at->domain)
        {
            case DOMAIN_INTEGER:
                if (w->v.i_val == v->v.i_val) 
                {
                    dml_update_cur(&cur, values);
                }
                break;
            case DOMAIN_FLOAT:
                if (w->v.f_val == v->v.f_val) 
                {
                    dml_update_cur(&cur, values);
                }
                break;
            case DOMAIN_VARCHAR:
                if (strcmp(w->v.v_val, v->v.v_val) == 0) 
                {
                    dml_update_cur(&cur, values);
                }
                break;
        }

        d_hfree(v);
    }

    dql_cursor_close(&cur);

    return 0;
}


void dql_cursor_create(struct dql_cursor *cur, char *rname)
{
    cur->rname = rname;
}

int dql_cursor_open(struct dql_cursor *cur)
{
    struct dd_rel_m *r;

    if ( (r = dd_get(cur->rname)) == 0)
    {
        return E_REL_NOT_FOUND;
    }

    cur->r = r;
    cur->b = 0;
    f_it(&r->f, &cur->it);

    return 0;
}

int dql_cursor_fetch(struct dql_tuple *t, struct dql_cursor *cur)
{
    char *blk;

    if (f_itnext(&cur->it))
    {
        blk = b_get(cur->it.f->fd, cur->it.b);

        t->t = blk_record(blk, cur->it.r);

        t->desc = &cur->r->desc;

        if (cur->b)
            b_unp(cur->b);

        cur->b = blk;
        b_pin(cur->b);

        b_put(blk);
        return 0;
    }
    else
    {
        return 1;
    }

}

void dql_cursor_close(struct dql_cursor *cur)
{
    if (cur->b)
        b_unp(cur->b);
}

