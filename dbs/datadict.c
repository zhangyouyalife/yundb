#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "datadict.h"
#include "file.h"

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

