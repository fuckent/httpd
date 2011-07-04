#ifndef DO_PARENT_H
#define DO_PARENT_H 	1

#include "inc.h"
#include "do_child.h"

#define NSEC_DELAY		100000000

httpd_return_t	httpd_do_parent();
httpd_return_t	httpd_kill_all_child();

#endif
