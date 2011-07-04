#include "debug.h"

httpd_return_t
httpd_debug_config()
{
#ifdef LOG_FILE
	freopen(LOG_FILE, "w+", stdout);
#endif	
	return SUCCESS;
}
