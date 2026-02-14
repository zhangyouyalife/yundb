#ifndef _SYSFILE_
#define _SYSFILE_

#include "conf.h"
#include "exitcode.h"

int sf_blks(int fd);
void sf_wb(int fd, int bn, char bd[BLK_SZ]);
void sf_rb(int fd, int bn, char bd[BLK_SZ]);

#endif

