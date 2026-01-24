#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exitcode.h"
#include "db.h"
#include "file.h"
#include "datadict.h"

void db_rel(char *name, int nattr, int forg, char *rec);

void db_attr(char *rname, char *aname, int domain, 
        int pos, int len, char *rec);

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

    /* init relation.rel */
    strcpy(tpath, path);
    strcat(tpath, "relation.rel");
    f_crt(&relation, tpath);

    f_open(&relation, tpath);

    f_binit(dblk);

    r = f_nr(dblk, DD_REL_RSZ(REL_NAME));
    db_rel(REL_NAME, 3, FO_HEAP, r); 

    f_wb(&relation, 1, dblk);
    relation.hdr->blks++;
    f_close(&relation);

    /* init attribute */
    strcpy(tpath, path);
    strcat(tpath, ATTR_NAME);
    strcat(tpath, ".rel");
    f_crt(&attribute, tpath);

    f_open(&attribute, tpath);

    f_binit(dblk);

    /* attr: relation.name */
    r = f_nr(dblk, DD_ATTR_RSZ(REL_NAME, "name"));
    db_attr(REL_NAME, "name", DOMAIN_VARCHAR, 0, 256, r);

    /* attr: relation.nattr */
    r = f_nr(dblk, DD_ATTR_RSZ(REL_NAME, "nattr"));
    db_attr(REL_NAME, "nattr", DOMAIN_INTEGER, 1, 2, r);

    /* attr: relation.forg */
    r = f_nr(dblk, DD_ATTR_RSZ(REL_NAME, "forg"));
    db_attr(REL_NAME, "forg", DOMAIN_INTEGER, 2, 1, r);

    f_wb(&attribute, 1, dblk);
    attribute.hdr->blks++;
    f_close(&attribute);
}

/* attribte record */
void db_attr(char *rname, char *aname, int domain, int pos, int len, char *rec)
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
void db_rel(char *name, int nattr, int forg, char *rec)
{
    struct dd_rel *r;

    r = (struct dd_rel *) rec;
    r->name.off = sizeof(struct dd_rel);
    r->name.len = strlen(name);
    r->nattr = nattr;
    r->forg = forg;
    strncpy(rec + r->name.off, name, strlen(name)); 
}

