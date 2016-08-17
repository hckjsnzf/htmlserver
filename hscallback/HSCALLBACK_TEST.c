#include "htmlserver.h"

funcHSProcessHttpRequest HSProcessHttpRequest ;
int HSProcessHttpRequest( struct HttpUri *p_httpuri , struct HttpEnv *p_httpenv )
{
	struct HttpBuffer	*p_http_reqbuf = NULL ;
	struct HttpBuffer	*p_http_rspbuf = NULL ;
	
	int			nret = 0 ;
	
	p_http_reqbuf = GetHttpRequestBuffer(p_httpenv) ;
	p_http_rspbuf = GetHttpResponseBuffer(p_httpenv) ;
	
	nret = StrcatfHttpBuffer( p_http_rspbuf ,
		"Server: htmlserver/%s(%s)\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 22\r\n"
		"\r\n"
		"hello HSCALLBACK_TEST\n"
		, __HTMLSERVER_VERSION , __FILE__ ) ;
	if( nret )
		return HTTP_INTERNAL_SERVER_ERROR;
	else
		return HTTP_OK;
}

