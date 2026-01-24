#include "datadict.h"

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

