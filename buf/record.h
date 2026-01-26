#ifndef _RECORD_
#define _RECORD_

#include <unistd.h>

struct __attribute__((packed)) VarField
{
    int16_t vf_off;
    int16_t vf_len;
};

struct __attribute__((packed)) Record
{
    struct VarField id;
    struct VarField name;
    struct VarField dept_name;
    double salary;
};

#endif

