/*
 * htmlserver - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_HTMLSERVER_IN_
#define _H_HTMLSERVER_IN_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/inotify.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <dirent.h>
#define __USE_GNU
#include <sched.h>
#include <pthread.h>
#include <dlfcn.h>

#include "fasterhttp.h"
#include "LOGC.h"
#include "rbtree.h"
#include "list.h"

#include "IDL_htmlserver_conf.dsc.h"

#include "htmlserver.h"

#define HTMLSERVER_LISTEN_SOCKFDS		"HTMLSERVER_LISTEN_SOCKFDS"	/* 环境变量名，用于优雅重启时传给下一辈侦听信息 */

#define MAX_EPOLL_EVENTS			10000	/* 每次从epoll取回事件数量 */

#define INIT_HTTP_SESSION_COUNT			100	/* 初始化HTTP通讯会话数量 */
#define INCRE_HTTP_SESSION_COUNT		100	/* 每次补充的HTTP通讯会话数量 */
#define MAX_HTTP_SESSION_COUNT_DEFAULT		100000	/* 缺省的最大HTTP通讯会话数量 */

/* 会话类型 */
#define DATASESSION_TYPE_PIPE			'P'
#define DATASESSION_TYPE_LISTEN			'L'
#define DATASESSION_TYPE_HTMLCACHE		'C'
#define DATASESSION_TYPE_HTTP			'A'

#define SIGNAL_REOPEN_LOG			'L' /* 重新打开日志 */

/* 网络信息结构 */
struct NetAddress
{
	char			ip[ 15 + 1 ] ;
	int			port ;
	SOCKET			sock ;
	struct sockaddr_in	addr ;
} ;

/* 泛数据会话结构，type为其它会话做预判断 */
struct DataSession
{
	char			type ;
} ;

/* 侦听会话结构 */
struct ListenSession
{
	char			type ;
	
	struct NetAddress	netaddr ;
	
	struct list_head	list ;
} ;

/* 流类型结构 */
struct MimeType
{
	char			type[ sizeof( ((htmlserver_conf*)0)->mime_types.mime_type[0].type ) ] ;
	char			mime[ sizeof( ((htmlserver_conf*)0)->mime_types.mime_type[0].mime ) ] ;
	
	int			type_len ;
	
	struct hlist_node	mimetype_node ;
} ;

/* 虚拟主机结构 */
struct VirtualHost
{
	char			domain[ sizeof( ((htmlserver_conf*)0)->servers.server[0].domain ) ] ;
	int			domain_len ;
	char			wwwroot[ sizeof( ((htmlserver_conf*)0)->servers.server[0].wwwroot ) ] ;
	char			index[ sizeof( ((htmlserver_conf*)0)->servers.server[0].index ) ] ;
	char			access_log[ sizeof( ((htmlserver_conf*)0)->servers.server[0].access_log ) ] ;
	
	int			access_log_fd ;
	
	struct hlist_node	virtualhost_node ;
} ;

/* 网页缓存会话结构 */
struct HtmlCacheSession
{
	char			type ;
	
	char			*pathfilename ;
	int			pathfilename_len ;
	struct rb_node		htmlcache_pathfilename_rbnode ;
	
	struct stat		st ;
	char			*html_content ;
	int			html_content_len ;
	char			*html_gzip_content ;
	int			html_gzip_content_len ;
	char			*html_deflate_content ;
	int			html_deflate_content_len ;
	
	int			wd ;
	struct rb_node		htmlcache_wd_rbnode ;
	
	struct list_head	list ;
} ;

/* HTTP通讯会话 */
struct HttpSession
{
	char			type ;
	
	struct NetAddress	netaddr ;
	struct HttpEnv		*http ;
	
	int			timeout_timestamp ;
	struct rb_node		timeout_rbnode ;
	
	struct list_head	list ;
} ;

/* 工作进程共享信息结构 */
struct ProcessInfo
{
	int			pipe[2] ;
	
	pid_t			pid ;
	
	int			epoll_fd ;
	int			epoll_nfds ;
} ;

struct HtmlServer ;

#define HSPROCESSHTTPREQUEST		"HSProcessHttpRequest"

/* 主环境结构 */
struct HtmlServer
{
	char				**argv ;
	char				config_pathfilename[ 256 + 1 ] ;
	htmlserver_conf			*p_config ;
	int				log_level ;
	
	int				process_info_shmid ;
	struct ProcessInfo		*process_info_array ;
	struct ProcessInfo		*p_this_process_info ;
	int				process_info_index ;
	
	struct VirtualHost		*p_virtualhost_default ;
	int				virtualhost_hashsize ;
	int				virtualhost_count ;
	struct hlist_head		*virtualhost_hash ;
	struct VirtualHost		*p_virtualhost ;
	
	int				mimetype_hashsize ;
	struct hlist_head		*mimetype_hash ;
	
	int				hscallback_type_len ;
	void				*dlHSProcessHttpRequest ;
	funcHSProcessHttpRequest	*pfuncHSProcessHttpRequest ;
	
	struct DataSession		pipe_session ;
	
	struct ListenSession		listen_session_list ;
	int				listen_session_count ;
	
	int				htmlcache_inotify_fd ;
	struct HtmlCacheSession		htmlcache_session ;
	struct HtmlCacheSession		htmlcache_session_list ;
	int				htmlcache_session_count ;
	struct rb_root			htmlcache_wd_rbtree ;
	struct rb_root			htmlcache_pathfilename_rbtree ;
	
	int				http_session_used_count ;
	struct HttpSession		http_session_unused_list ;
	int				http_session_unused_count ;
	
	struct rb_root			http_session_rbtree_used ;
} ;

extern struct HtmlServer	*g_p_server ;
extern signed char		g_second_elapse ;

extern char			*__HTMLSERVER_VERSION ;

int InitMimeTypeHash( struct HtmlServer *p_server );
void CleanMimeTypeHash( struct HtmlServer *p_server );
int PushMimeTypeHashNode( struct HtmlServer *p_server , struct MimeType *p_mimetype );
struct MimeType *QueryMimeTypeHashNode( struct HtmlServer *p_server , char *type , int type_len );

int InitVirtualHostHash( struct HtmlServer *p_server );
void CleanVirtualHostHash( struct HtmlServer *p_server );
int PushVirtualHostHashNode( struct HtmlServer *p_server , struct VirtualHost *p_virtualhost );
struct VirtualHost *QueryVirtualHostHashNode( struct HtmlServer *p_server , char *domain , int domain_len );

int AddHtmlCacheWdTreeNode( struct HtmlServer *p_server , struct HtmlCacheSession *p_htmlcache_session );
struct HtmlCacheSession *QueryHtmlCacheWdTreeNode( struct HtmlServer *p_server , int wd );
void RemoveHtmlCacheWdTreeNode( struct HtmlServer *p_server , struct HtmlCacheSession *p_htmlcache_session );

int AddHtmlCachePathfilenameTreeNode( struct HtmlServer *p_server , struct HtmlCacheSession *p_htmlcache_session );
struct HtmlCacheSession *QueryHtmlCachePathfilenameTreeNode( struct HtmlServer *p_server , char *pathfilename );
void RemoveHtmlCachePathfilenameTreeNode( struct HtmlServer *p_server , struct HtmlCacheSession *p_htmlcache_session );

int AddHttpSessionTimeoutTreeNode( struct HtmlServer *p_server , struct HttpSession *p_http_session );
void RemoveHttpSessionTimeoutTreeNode( struct HtmlServer *p_server , struct HttpSession *p_http_session );
int UpdateHttpSessionTimeoutTreeNode( struct HtmlServer *p_server , struct HttpSession *p_http_session , int timeout_timestamp );
struct HttpSession *GetExpireHttpSessionTimeoutTreeNode( struct HtmlServer *p_server );

int InitServerEnvirment( struct HtmlServer *p_server );
void CleanServerEnvirment( struct HtmlServer *p_server );
int SaveListenSockets( struct HtmlServer *p_server );
int LoadOldListenSockets( struct NetAddress **pp_old_netaddr_array , int *p_old_netaddr_array_count );
struct NetAddress *GetListener( struct NetAddress *old_netaddr_array , int old_netaddr_array_count , char *ip , int port );
int CloseUnusedOldListeners( struct NetAddress *p_old_netaddr_array , int old_netaddr_array );

int IncreaseHttpSessions( struct HtmlServer *p_server , int http_session_incre_count );
struct HttpSession *FetchHttpSessionUnused( struct HtmlServer *p_server );
void SetHttpSessionUnused( struct HtmlServer *p_server , struct HttpSession *p_http_session );

void FreeHtmlCacheSession( struct HtmlCacheSession *p_htmlcache_session );

int OnSendingSocket( struct HtmlServer *p_server , struct HttpSession *p_http_session );
int OnReceivingSocket( struct HtmlServer *p_server , struct HttpSession *p_http_session );
int OnAcceptingSocket( struct HtmlServer *p_server , struct ListenSession *p_listen_session );

int DirectoryWatcherEventHander( struct HtmlServer *p_server );
int HtmlCacheEventHander( struct HtmlServer *p_server );

void *TimerThread( void *pv );

int WorkerProcess( void *pv );

int MonitorProcess( void *pv );

int LoadConfig( char *config_pathfilename , struct HtmlServer *p_server );

int BindDaemonServer( int (* ServerMain)( void *pv ) , void *pv );
int AccessDirectoryExist( char *pathdirname );
int BindCpuAffinity( int processor_no );
unsigned long CalcHash( char *str , int len );

int ProcessHttpRequest( struct HtmlServer *p_server , struct HttpSession *p_http_session , char *pathname , char *filename , int filename_len );

#endif

