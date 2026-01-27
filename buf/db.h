#ifndef _DB_
#define _DB_

#include "datadict.h"


struct d_datum_h *db_attr_val(int pos, char *r, struct dd_reldesc *d);

struct ddl_attr
{
    char        *name;
    uint8_t     domain;
    uint16_t    pos;
    uint16_t    len;
};

struct ddl_create
{
    char *name;
    uint16_t nattr;
    uint8_t forg;
    struct ddl_attr *attrs;

};

void ddl_create_new(struct ddl_create *c, 
        char *name, uint16_t nattr, uint8_t forg);
void ddl_create_attr(struct ddl_create *c, char *name, uint16_t pos, 
        uint8_t domain, uint16_t len);
void ddl_create_go(struct ddl_create *c);
void ddl_create_free(struct ddl_create *c);


void ddl_drop(char *name);

struct dml_rec
{
    int16_t sz;
    char *r;
};

union dml_value
{
        int64_t i_val;
        double f_val;
        char *v_val;
};

struct dml_where {
    char *attr;
    union dml_value v;
};

void dml_r(struct dml_rec *r, 
        union dml_value *values, struct dd_reldesc *d);
void dml_rfree(struct dml_rec *r);

int dml_insert(char *rel, union dml_value *values);


int dml_delete(char *rel, struct dml_where *w);

int dml_update(char *rel, union dml_value *values, struct dml_where *w);

#define E_REL_NOT_FOUND 1
#define E_ATTR_NOT_FOUND 2

#endif
