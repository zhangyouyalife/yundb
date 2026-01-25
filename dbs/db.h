#ifndef _DB_
#define _DB_

#include "datadict.h"

char db_path[256];

void db_init(char *path);

void db_val(char *val, int pos, char *r, struct dd_reldesc *d); 

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

union db_value {
    int64_t i_val;
    double f_val;
    char *v_val;
};

int dml_insert(char *rel, union db_value *values);

#define E_REL_NOT_FOUND 1

#endif
