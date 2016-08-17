/*
 * htmlserver - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "htmlserver_in.h"

struct HtmlServer	*g_p_server = NULL ;

char	__HTMLSERVER_VERSION_1_0_0[] = "1.0.0" ;
char	*__HTMLSERVER_VERSION = __HTMLSERVER_VERSION_1_0_0 ;

static void usage()
{
	printf( "htmlserver v%s build %s %s\n" , __HTMLSERVER_VERSION , __DATE__ , __TIME__ );
	printf( "USAGE : htmlserver_in.htmlserver.conf\n" );
	return;
}

int main( int argc , char *argv[] )
{
	struct HtmlServer	server ;
	struct HtmlServer	*p_server = NULL ;
	
	int			nret = 0 ;
	
	if( argc == 1 + 1 )
	{
		/* ��������HTTP״̬�롢����Ϊȱʡ */
		ResetAllHttpStatus();
		
		p_server = & server ;
		g_p_server = p_server ;
		memset( p_server , 0x00 , sizeof(struct HtmlServer) );
		p_server->argv = argv ;
		
		/* ����ȱʡ����־ */
		SetLogFile( "%s/log/error.log" , getenv("HOME") );
		SetLogLevel( LOGLEVEL_ERROR );
		SETPID
		SETTID
		UPDATEDATETIMECACHEFIRST
		
		/* װ������ */
		strncpy( p_server->config_pathfilename , argv[1] , sizeof(p_server->config_pathfilename)-1 );
		p_server->p_config = (htmlserver_conf *)malloc( sizeof(htmlserver_conf) ) ;
		if( p_server->p_config == NULL )
		{
			if( getenv( HTMLSERVER_LISTEN_SOCKFDS ) == NULL )
				printf( "alloc failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
		}
		memset( p_server->p_config , 0x00 , sizeof(htmlserver_conf) );
		nret = LoadConfig( p_server->config_pathfilename , p_server ) ;
		if( nret )
		{
			if( getenv( HTMLSERVER_LISTEN_SOCKFDS ) == NULL )
				printf( "Load config failed[%d]\n" , nret );
			return -nret;
		}
		
		/* ������������־ */
		CloseLogFile();
		
		SetLogFile( p_server->p_config->error_log );
		SetLogLevel( p_server->log_level );
		SETPID
		SETTID
		UPDATEDATETIMECACHEFIRST
		InfoLog( __FILE__ , __LINE__ , "--- htmlserver v%s build %s %s ---" , __HTMLSERVER_VERSION , __DATE__ , __TIME__ );
		SetHttpCloseExec( g_file_fd );
		
		/* ��ʼ������������ */
		nret = InitServerEnvirment( p_server ) ;
		if( nret )
		{
			if( getenv( HTMLSERVER_LISTEN_SOCKFDS ) == NULL )
				printf( "Init envirment failed[%d]\n" , nret );
			return -nret;
		}
		
		return -BindDaemonServer( & MonitorProcess , p_server );
		/*
		return -MonitorProcess( p_server );
		*/
	}
	else
	{
		usage();
		exit(9);
	}
}

