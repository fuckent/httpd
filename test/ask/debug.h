#ifndef DEBUG_H
#define DEBUG_H

#include "inc.h"

#define DEBUG_MODE		1
//#define LOG_FILE		"log.txt"

httpd_return_t httpd_debug_config();

#ifndef DEBUG_MODE
	#undef MSG
	#define MSG(msg)	{}
#endif


#endif
