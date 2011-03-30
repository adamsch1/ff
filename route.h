#ifndef __ROUTE__H
#define __ROUTE__H

#include "ff.h"

void route_import_controllers( const char *path );
int route_dispatch(char *uri);

#endif
