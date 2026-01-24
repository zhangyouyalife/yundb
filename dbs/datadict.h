#ifndef _DATADICT_
#define _DATADICT_

#define FO_HEAP 1
#define FO_SEQ  2
#define FO_HASH 3

#include <stdint.h>
#include <string.h>

struct __attribute__((packed)) dd_va
{
    int16_t off;
    int16_t len;
};

struct __attribute__((packed)) dd_rel
{
    struct dd_va    name;
    uint16_t        nattr;
    uint8_t        forg;
    char            va_start[0];
};

#define DD_REL_RSZ(name)    (sizeof(struct dd_rel) + strlen(name))

#define REL_NAME        "relation"
#define REL_NAME_LEN    8

struct __attribute__((packed)) dd_attr
{
    struct dd_va    rel;
    struct dd_va    attr;
    uint8_t         domain;
    uint16_t        pos;
    uint16_t        len;
};

#define DD_ATTR_RSZ(r, a)   (sizeof(struct dd_attr) \
                                + strlen(r) \
                                + strlen(a))

#define ATTR_NAME       "attribute"
#define ATTR_NAME_LEN   sizeof(ATTR_NAME)

#define DOMAIN_INTEGER  1
#define DOMAIN_FLOAT    2
#define DOMAIN_VARCHAR  3

void dd_rel(char *name, int nattr, int forg, char *rec);

void dd_attr(char *rname, char *aname, int domain, 
        int pos, int len, char *rec);

#endif

