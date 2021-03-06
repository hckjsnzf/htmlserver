/*
 * htmlserver - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "htmlserver_in.h"

int IncreaseHttpSessions( struct HtmlServer *p_server , int http_session_incre_count )
{
	struct HttpSession	*p_http_session_array = NULL ;
	struct HttpSession	*p_http_session = NULL ;
	int			i ;
	
	/* 判断是否到达最大HTTP通讯会话数量 */
	if( p_server->http_session_used_count >= p_server->p_config->limits.max_http_session_count )
	{
		WarnLog( __FILE__ , __LINE__ , "http session count limits[%d]" , p_server->p_config->limits.max_http_session_count );
		return 1;
	}
	
	/* 批量增加空闲HTTP通讯会话 */
	if( p_server->http_session_used_count + http_session_incre_count > p_server->p_config->limits.max_http_session_count )
		http_session_incre_count = p_server->p_config->limits.max_http_session_count - p_server->http_session_used_count ;
	
	p_http_session_array = (struct HttpSession *)malloc( sizeof(struct HttpSession) * http_session_incre_count ) ;
	if( p_http_session_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "malloc failed , errno[%d]" , errno );
		return -1;
	}
	memset( p_http_session_array , 0x00 , sizeof(struct HttpSession) * http_session_incre_count );
	
	for( i = 0 , p_http_session = p_http_session_array ; i < http_session_incre_count ; i++ , p_http_session++ )
	{
		p_http_session->http = CreateHttpEnv() ;
		SetHttpTimeout( p_http_session->http , p_server->p_config->http_options.timeout );
		list_add_tail( & (p_http_session->list) , & (p_server->http_session_unused_list.list) );
		DebugLog( __FILE__ , __LINE__ , "init http session[%p] http env[%p]" , p_http_session , p_http_session->http );
	}
	
	p_server->http_session_unused_count += http_session_incre_count ;
	
	return 0;
}

struct HttpSession *FetchHttpSessionUnused( struct HtmlServer *p_server )
{
	struct HttpSession	*p_http_session = NULL ;
	
	int			nret = 0 ;
	
	if( p_server->http_session_unused_count == 0 )
	{
		/* 如果空闲HTTP通讯会话链表为空 */
		nret = IncreaseHttpSessions( p_server , INCRE_HTTP_SESSION_COUNT ) ;
		if( nret )
			return NULL;
	}
	
	/* 从空闲HTTP通讯会话链表中移出一个会话，并返回之 */
	p_http_session = list_first_entry( & (p_server->http_session_unused_list.list) , struct HttpSession , list ) ;
	list_del( & (p_http_session->list) );
	
	/* 加入到工作HTTP通讯会话树中 */
	p_http_session->timeout_timestamp = GETSECONDSTAMP + p_server->p_config->http_options.timeout ;
	nret = AddHttpSessionTimeoutTreeNode( p_server , p_http_session ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AddTimeoutTreeNode failed , errno[%d]" , errno );
		list_add_tail( & (p_http_session->list) , & (p_server->http_session_unused_list.list) );
		return NULL;
	}
	
	/* 调整计数器 */
	p_server->http_session_used_count++;
	p_server->http_session_unused_count--;
	DebugLog( __FILE__ , __LINE__ , "fetch http session[%p] http env[%p]" , p_http_session , p_http_session->http );
	
	return p_http_session;
}

void SetHttpSessionUnused( struct HtmlServer *p_server , struct HttpSession *p_http_session )
{
	DebugLog( __FILE__ , __LINE__ , "reset http session[%p] http env[%p]" , p_http_session , p_http_session->http );
	
	/* 清理HTTP通讯会话 */
	epoll_ctl( p_server->p_this_process_info->epoll_fd , EPOLL_CTL_DEL , p_http_session->netaddr.sock , NULL );
	close( p_http_session->netaddr.sock );
	ResetHttpEnv( p_http_session->http );
	
	/* 把当前工作HTTP通讯会话移到空闲HTTP通讯会话链表中 */
	RemoveHttpSessionTimeoutTreeNode( p_server , p_http_session );
	list_add_tail( & (p_http_session->list) , & (p_server->http_session_unused_list.list) );
	
	p_server->http_session_used_count--;
	p_server->http_session_unused_count++;
	
	return;
}

