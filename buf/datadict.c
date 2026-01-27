#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "exitcode.h"
#include "list.h"
#include "datadict.h"
#include "data.h"
#include "file.h"

static struct dbf  relation;

static struct dbf  attribute;

/* attribte record */
void dd_attr(char *rname, char *aname, int domain, int pos, int len, char *rec)
{
    struct dd_attr *a;
    int lr, la;

    lr = strlen(rname);
    la = strlen(aname);

    a = (struct dd_attr *) rec;
    a->rel.off = sizeof(struct dd_attr);
    a->rel.len = lr;
    a->attr.off = a->rel.off + a->rel.len;
    a->attr.len = la;
    a->domain = domain;
    a->pos = pos;
    a->len = len;
    strncpy((char *)a + a->rel.off, rname, lr); 
    strncpy((char *)a + a->attr.off, aname, la);
} 

/* relation record */
void dd_rel(char *name, int nattr, int forg, char *rec)
{
    struct dd_rel *r;

    r = (struct dd_rel *) rec;
    r->name.off = sizeof(struct dd_rel);
    r->name.len = strlen(name);
    r->nattr = nattr;
    r->forg = forg;
    strncpy(rec + r->name.off, name, strlen(name)); 
}


void dd_reldescfrom(struct dd_reldesc *m, struct dd_rel *d)
{
    f_strcpy(m->name, &d->name, (char *)d);
    m->nattr = d->nattr;
    m->forg = d->forg;
}

void dd_attrdescfrom(struct dd_attrdesc *m, struct dd_attr *d)
{
    f_strcpy(m->name, &d->attr, (char *)d); 
    m->domain = d->domain;
    m->pos = d->pos;
    m->len = d->len;
}

int dd_attrdesc_get(struct dd_attrdesc ads[], char *rname)
{
    struct dbf_it it;
    char *r;
    struct dd_attr *attr;
    static char nm[NAME_MAXSZ];
    int found;

    f_it(&attribute, &it);

    found = 0;
    while ( (r = f_itnext(&it)) != 0 ) 
    {
        attr = (struct dd_attr *) r;
        f_strcpy(nm, &attr->rel, r);
        if (strcmp(nm, rname) == 0)
        {
            found++;
            dd_attrdescfrom(&ads[attr->pos], attr);
        }
    }
    
    f_itfree(&it);

    return found;
}

int dd_reldesc_get(struct dd_reldesc *rd, char *name)
{
    struct dbf_it it;
    char *r;
    struct dd_rel *rel;
    static char nb[NAME_MAXSZ];
    int found;

    f_it(&relation, &it);

    found = 0;
    while ( (r = f_itnext(&it)) != 0 ) 
    {
        rel = (struct dd_rel *) r;
        f_strcpy(nb, &rel->name, r);
        if (strcmp(nb, name) == 0)
        {
            dd_reldescfrom(rd, rel);
            found = 1;

            if (rd->nattr == 0)
            {
                rd->attrs = 0;
            }
            else
            {
                rd->attrs = malloc(rd->nattr * sizeof(struct dd_attrdesc));
                /*printf("has %d attrs\n", rd->nattr);*/

                dd_attrdesc_get(rd->attrs, name);
            }
            break;
        }
    }
    
    f_itfree(&it);

    return found;
}

void dd_reldesc_free(struct dd_reldesc *d)
{
    free(d->attrs);
}

/* get attr desc by name */
struct dd_attrdesc *dd_reldesc_attr(char *name, struct dd_reldesc *d)
{
    struct dd_attrdesc *a, *at;
    int i;

    at = 0;
    for (i = 0; i < d->nattr; i++)
    {
        a = &d->attrs[i];
        if (strcmp(a->name, name) == 0)
        {
            at = a;
            break;
        }
    }

    return at;
}


void dd_crt_relation();

void dd_crt_attribute();

void dd_create(char *path)
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

    dd_crt_relation();

    dd_crt_attribute();

    f_close(&attribute);
    f_close(&relation);

}

void dd_crt_attribute()
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

void dd_crt_relation()
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

struct dd_rel_m *dd_relmget(char *rname)
{
    struct dd_rel_m *r;
    char fn[256];

    if ( (r = malloc(sizeof(struct dd_rel_m))) == 0)
    {
        perror("dd_relmget malloc");
        exit(EC_M);
    }

    dd_reldesc_get(&r->desc, rname);

    sprintf(fn, "%s%s.rel", db_path, rname);
    f_open(&r->f, fn);

    return r;
}
void dd_relmfree(void *relm)
{
    struct dd_rel_m *r;

    r = (struct dd_rel_m *) relm;

    dd_reldesc_free(&r->desc);

    f_close(&r->f);

    free(relm);
}

/* data dict init 
 * DO NOT USE BUFFER layer, for it has not initialzied */
void dd_init()
{
    struct dd_reldesc rd, *d;
    struct dd_attrdesc *ad;
    struct dbf rf, *f;
    struct dbf_it it;
    char s[256];
    int i;
    char *r;
    struct dd_rel_m *m;
    struct d_datum_h *ddh;
    struct d_datum_b ddb;
    int off;
    struct dbf_va *va;

    ll_init(&datadict);

    sprintf(s, "%s%s.rel", db_path, "relation");
    f_open(&relation, s);
    sprintf(s, "%s%s.rel", db_path, "attribute");
    f_open(&attribute, s);

    dd_reldesc_get(&rd, REL_NAME);

    sprintf(s, "%s%s.rel", db_path, REL_NAME);
    f_open(&rf, s);
    f_it(&rf, &it);

    while ( (r = f_itnext(&it)) != 0)
    {
        for (i = 0; i < rd.nattr; i++)
        {
            ad = &rd.attrs[i];
            if (0 == strcmp(ad->name, "name"))
            {
                /* TODO refactor db_val */ 
                
                off = dd_attr_off(ad->pos, &rd);
                va = (struct dbf_va *) (r + off);

                ddb.domain = ad->domain;
                ddb.len = va->len;
                ddb.bytes = r + va->off;

                ddh = d_btoh(&ddb);
                ll_add(&datadict, dd_relmget(ddh->v.v_val)); 
                d_hfree(ddh);
            }
        }
    }

    f_itfree(&it);
    f_close(&rf);
    dd_reldesc_free(&rd);

    f_close(&attribute);
    f_close(&relation);
}

void dd_free()
{
    ll_travel(&datadict, dd_relmfree);
    ll_free(&datadict);
}

struct dd_rel_m *dd_get(char *rname)
{
    struct linknode *n;
    struct dd_rel_m *r;

    n = datadict.first;
    while (n != 0)
    {
        r = (struct dd_rel_m *) n->data;
        if (strcmp(rname, r->desc.name) == 0)
        {
            return r;
        }

        n = n->next;
    }

    return 0;
}

int dd_attr_off(int pos, struct dd_reldesc *d)
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

