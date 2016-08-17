/*
 * htmlserver - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "htmlserver_in.h"

char *strndup(const char *s, size_t n);

/* ���ַ����е�$...$�û��������滻 */
static int StringExpandEnvval( char *buf , int buf_size )
{
	int		total_len ;
	char		*p1 = NULL ;
	char		*p2 = NULL ;
	char		*env_name = NULL ;
	char		*env_value = NULL ;
	int		env_value_len ;
	
	total_len = strlen( buf ) ;
	p1 = buf ;
	while(1)
	{
		p1 = strchr( p1 , '$' ) ;
		if( p1 == NULL )
			break;
		p2 = strchr( p1+1 , '$' ) ;
		if( p2 == NULL )
			break;
		
		env_name = strndup( p1+1 , p2-p1-1 ) ;
		if( env_name == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "strndup failed , errno[%d]" , errno );
			return -1;
		}
		
		env_value = getenv( env_name ) ;
		if( env_value == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "getenv[%s] failed , errno[%d]" , env_name );
			free( env_name );
			return -1;
		}
		
		/*
		p    p
		1    2
		$HOME$/log/access.log
		/home/calvin/log/access.log
		*/
		env_value_len = strlen( env_value ) ;
		if( total_len + ( env_value_len - (p2-p1+1) ) > buf_size-1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "buf[%s] replace overflow" , buf );
			free( env_name );
			return -1;
		}
		memmove( p1+env_value_len , p2+1 , strlen(p2+1)+1 );
		memcpy( p1 , env_value , env_value_len );
		
		free( env_name );
	}
	
	return 0;
}

/* ���ַ����͵���־�ȼ�ת��Ϊ���� */
static int ConvertLogLevel_atoi( char *log_level_desc , int *p_log_level )
{
	if( strcmp( log_level_desc , "DEBUG" ) == 0 )
		(*p_log_level) = LOGLEVEL_DEBUG ;
	else if( strcmp( log_level_desc , "INFO" ) == 0 )
		(*p_log_level) = LOGLEVEL_INFO ;
	else if( strcmp( log_level_desc , "WARN" ) == 0 )
		(*p_log_level) = LOGLEVEL_WARN ;
	else if( strcmp( log_level_desc , "ERROR" ) == 0 )
		(*p_log_level) = LOGLEVEL_ERROR ;
	else if( strcmp( log_level_desc , "FATAL" ) == 0 )
		(*p_log_level) = LOGLEVEL_FATAL ;
	else
		return -11;
	
	return 0;
}

/* װ�������ļ� */
/* ���������ڴ棬��ע������ͷ� */
static char *StrdupEntireFile( char *pathfilename , int *p_file_size )
{
	struct stat	st ;
	char		*p_html_content = NULL ;
	FILE		*fp = NULL ;
	int		nret = 0 ;
	
	nret = stat( pathfilename , & st ) ;
	if( nret == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "stat[%s] failed , errno[%d]" , pathfilename , errno );
		return NULL;
	}
	
	p_html_content = (char*)malloc( st.st_size+1 ) ;
	if( p_html_content == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "malloc failed , errno[%d]" , errno );
		return NULL;
	}
	memset( p_html_content , 0x00 , st.st_size+1 );
	
	fp = fopen( pathfilename , "r" ) ;
	if( fp == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "fopen[%s] failed , errno[%d]" , pathfilename , errno );
		return NULL;
	}
	
	nret = fread( p_html_content , st.st_size , 1 , fp ) ;
	if( nret != 1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "fread failed , errno[%d]" , errno );
		return NULL;
	}
	
	fclose( fp );
	
	if( p_file_size )
		(*p_file_size) = (int)(st.st_size) ;
	return p_html_content;
}

int LoadConfig( char *config_pathfilename , struct HtmlServer *p_server )
{
	char		*buf = NULL ;
	int		file_size ;
	int		i ;
	
	int		nret = 0 ;
	
	/* ��ȡ�����ļ� */
	buf = StrdupEntireFile( config_pathfilename , & file_size ) ;
	if( buf == NULL )
		return -1;
	
	/* Ԥ��ȱʡֵ */
	p_server->p_config->worker_processes = sysconf(_SC_NPROCESSORS_ONLN) ;
	
	p_server->p_config->limits.max_http_session_count = MAX_HTTP_SESSION_COUNT_DEFAULT ;
	
	/* ���������ļ� */
	nret = DSCDESERIALIZE_JSON_htmlserver_conf( NULL , buf , & file_size , p_server->p_config ) ;
	free( buf );
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_htmlserver_conf failed[%d][%d] , errno[%d]errline[%d]" , nret , DSCGetErrorLine_htmlserver_conf() , errno , DSCGetErrorLine_htmlserver_conf() );
		return -1;
	}
	
	if( p_server->p_config->worker_processes <= 0 )
	{
		p_server->p_config->worker_processes = sysconf(_SC_NPROCESSORS_ONLN) ;
	}
	
	/* չ������־���еĻ������� */
	nret = StringExpandEnvval( p_server->p_config->error_log , sizeof(p_server->p_config->error_log) ) ;
	if( nret )
		return nret;
	
	/* ת������־�ȼ�ֵ */
	nret = ConvertLogLevel_atoi( p_server->p_config->log_level , &(p_server->log_level) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "log_level[%s] invalid" , p_server->p_config->log_level );
		return nret;
	}
	
	/* չ���ص�ģ���ļ��� */
	nret = StringExpandEnvval( p_server->p_config->hscallback_sopathfilename , sizeof(p_server->p_config->hscallback_sopathfilename) ) ;
	if( nret )
		return nret;
	
	/* չ����־�ļ����еĻ������� */
	if( p_server->p_config->server.domain[0] == '\0' )
	{
		ErrorLog( __FILE__ , __LINE__ , "domain[%s] not found" , p_server->p_config->server.domain );
		return -1;
	}
	
	nret = StringExpandEnvval( p_server->p_config->server.wwwroot , sizeof(p_server->p_config->server.wwwroot) ) ;
	if( nret )
		return nret;
	
	nret = AccessDirectoryExist( p_server->p_config->server.wwwroot ) ;
	if( nret != 1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "wwwroot[%s] not exist" , p_server->p_config->server.wwwroot , nret );
		return -1;
	}
	
	nret = StringExpandEnvval( p_server->p_config->server.access_log , sizeof(p_server->p_config->server.access_log) ) ;
	if( nret )
		return nret;
	
	for( i = 0 ; i < p_server->p_config->servers._server_count ; i++ )
	{
		if( p_server->p_config->servers.server[i].domain[0] == '\0' )
		{
			ErrorLog( __FILE__ , __LINE__ , "domain[%s] not found" , p_server->p_config->servers.server[i].domain );
			return -1;
		}
		
		nret = StringExpandEnvval( p_server->p_config->servers.server[i].wwwroot , sizeof(p_server->p_config->servers.server[i].wwwroot) ) ;
		if( nret )
			return nret;
		
		nret = AccessDirectoryExist( p_server->p_config->servers.server[i].wwwroot ) ;
		if( nret != 1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "wwwroot[%s] not exist" , p_server->p_config->servers.server[i].wwwroot , nret );
			return -1;
		}
		
		nret = StringExpandEnvval( p_server->p_config->servers.server[i].access_log , sizeof(p_server->p_config->servers.server[i].access_log) ) ;
		if( nret )
			return nret;
	}
	
	/* ���ø��Ի�ҳ����Ϣ */
	if( STRCMP( p_server->p_config->error_pages.error_page_400 , != , "" ) )
	{
		char	*p_html_content = NULL ;
		
		nret = StringExpandEnvval( p_server->p_config->error_pages.error_page_400 , sizeof(p_server->p_config->error_pages.error_page_400) ) ;
		if( nret )
			return nret;
		
		p_html_content = StrdupEntireFile( p_server->p_config->error_pages.error_page_400 , NULL ) ;
		if( p_html_content == NULL )
			return -1;
		
		SetHttpStatus( HTTP_BAD_REQUEST , HTTP_BAD_REQUEST_S , p_html_content );
	}
	
	if( STRCMP( p_server->p_config->error_pages.error_page_401 , != , "" ) )
	{
		char	*p_html_content = NULL ;
		
		nret = StringExpandEnvval( p_server->p_config->error_pages.error_page_401 , sizeof(p_server->p_config->error_pages.error_page_401) ) ;
		if( nret )
			return nret;
		
		p_html_content = StrdupEntireFile( p_server->p_config->error_pages.error_page_401 , NULL ) ;
		if( p_html_content == NULL )
			return -1;
		
		SetHttpStatus( HTTP_UNAUTHORIZED , HTTP_UNAUTHORIZED_S , p_html_content );
	}
	
	if( STRCMP( p_server->p_config->error_pages.error_page_403 , != , "" ) )
	{
		char	*p_html_content = NULL ;
		
		nret = StringExpandEnvval( p_server->p_config->error_pages.error_page_403 , sizeof(p_server->p_config->error_pages.error_page_403) ) ;
		if( nret )
			return nret;
		
		p_html_content = StrdupEntireFile( p_server->p_config->error_pages.error_page_403 , NULL ) ;
		if( p_html_content == NULL )
			return -1;
		
		SetHttpStatus( HTTP_FORBIDDEN , HTTP_FORBIDDEN_S , p_html_content );
	}
	
	if( STRCMP( p_server->p_config->error_pages.error_page_404 , != , "" ) )
	{
		char	*p_html_content = NULL ;
		
		nret = StringExpandEnvval( p_server->p_config->error_pages.error_page_404 , sizeof(p_server->p_config->error_pages.error_page_404) ) ;
		if( nret )
			return nret;
		
		p_html_content = StrdupEntireFile( p_server->p_config->error_pages.error_page_404 , NULL ) ;
		if( p_html_content == NULL )
			return -1;
		
		SetHttpStatus( HTTP_NOT_FOUND , HTTP_NOT_FOUND_S , p_html_content );
	}
	
	if( STRCMP( p_server->p_config->error_pages.error_page_408 , != , "" ) )
	{
		char	*p_html_content = NULL ;
		
		nret = StringExpandEnvval( p_server->p_config->error_pages.error_page_408 , sizeof(p_server->p_config->error_pages.error_page_408) ) ;
		if( nret )
			return nret;
		
		p_html_content = StrdupEntireFile( p_server->p_config->error_pages.error_page_408 , NULL ) ;
		if( p_html_content == NULL )
			return -1;
		
		SetHttpStatus( HTTP_REQUEST_TIMEOUT , HTTP_REQUEST_TIMEOUT_S , p_html_content );
	}
	
	if( STRCMP( p_server->p_config->error_pages.error_page_500 , != , "" ) )
	{
		char	*p_html_content = NULL ;
		
		nret = StringExpandEnvval( p_server->p_config->error_pages.error_page_500 , sizeof(p_server->p_config->error_pages.error_page_500) ) ;
		if( nret )
			return nret;
		
		p_html_content = StrdupEntireFile( p_server->p_config->error_pages.error_page_500 , NULL ) ;
		if( p_html_content == NULL )
			return -1;
		
		SetHttpStatus( HTTP_INTERNAL_SERVER_ERROR , HTTP_INTERNAL_SERVER_ERROR_S , p_html_content );
	}
	
	if( STRCMP( p_server->p_config->error_pages.error_page_503 , != , "" ) )
	{
		char	*p_html_content = NULL ;
		
		nret = StringExpandEnvval( p_server->p_config->error_pages.error_page_503 , sizeof(p_server->p_config->error_pages.error_page_503) ) ;
		if( nret )
			return nret;
		
		p_html_content = StrdupEntireFile( p_server->p_config->error_pages.error_page_503 , NULL ) ;
		if( p_html_content == NULL )
			return -1;
		
		SetHttpStatus( HTTP_SERVICE_UNAVAILABLE , HTTP_SERVICE_UNAVAILABLE_S , p_html_content );
	}
	
	if( STRCMP( p_server->p_config->error_pages.error_page_505 , != , "" ) )
	{
		char	*p_html_content = NULL ;
		
		nret = StringExpandEnvval( p_server->p_config->error_pages.error_page_505 , sizeof(p_server->p_config->error_pages.error_page_505) ) ;
		if( nret )
			return nret;
		
		p_html_content = StrdupEntireFile( p_server->p_config->error_pages.error_page_505 , NULL ) ;
		if( p_html_content == NULL )
			return -1;
		
		SetHttpStatus( HTTP_VERSION_NOT_SUPPORTED , HTTP_VERSION_NOT_SUPPORTED_S , p_html_content );
	}
	
	return 0;
}

