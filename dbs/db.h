#ifndef _DB_
#define _DB_

#include "datadict.h"

void db_init(char *path);

void db_val(char *val, int pos, char *r, struct dd_reldesc *d); 

#endif
