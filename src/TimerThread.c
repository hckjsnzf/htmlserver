/*
 * htmlserver - High Performance Web Server
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "htmlserver_in.h"

void *TimerThread( void *pv )
{
	/* ÿ��һ���ӣ�����д��־������ʱ�仺��������©�¼� */
	while(1)
	{
		UPDATEDATETIMECACHE
		
		sleep(1);
		
		g_second_elapse = 1 ;
	}
	
	pthread_exit(NULL);
}

