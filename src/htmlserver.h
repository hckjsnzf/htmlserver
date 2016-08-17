/*
 * htmlserver - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_HTMLSERVER_
#define _H_HTMLSERVER_

#include "fasterhttp.h"

extern char		*__HTMLSERVER_VERSION ;

typedef int funcHSProcessHttpRequest( struct HttpUri *p_httpuri , struct HttpEnv *p_httpenv );

#endif

