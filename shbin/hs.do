#!/bin/bash

usage()
{
	echo "USAGE : hs.do [ status | start | stop | kill | restart | restart_graceful | relog ]"
}

if [ $# -eq 0 ] ; then
	usage
	exit 9
fi

case $1 in
	status)
		ps -ef | grep -w htmlserver | grep -v grep | awk '{if($3=="1")print $0}'
		ps -ef | grep -w htmlserver | grep -v grep | awk '{if($3!="1")print $0}'
		;;
	start)
		PID=`ps -ef | grep -w htmlserver | grep -v grep | awk '{if($3=="1")print $2}'`
		if [ x"$PID" != x"" ] ; then
			echo "*** WARN : htmlserver existed"
			exit 1
		fi
		htmlserver ~/etc/htmlserver.conf
		while [ 1 ] ; do
			sleep 1
			PID=`ps -ef | grep -w htmlserver | grep -v grep | awk '{if($3=="1")print $2}'`
			if [ x"$PID" != x"" ] ; then
				break
			fi
		done
		echo "htmlserver start ok"
		hs.do status
		;;
	stop)
		hs.do status
		PID=`ps -ef | grep -w htmlserver | grep -v grep | awk '{if($3=="1")print $2}'`
		if [ x"$PID" = x"" ] ; then
			echo "*** WARN : htmlserver not existed"
			exit 1
		fi
		kill $PID
		while [ 1 ] ; do
			sleep 1
			PID=`ps -ef | grep -w htmlserver | grep -v grep | awk '{if($3=="1")print $2}'`
			if [ x"$PID" = x"" ] ; then
				break
			fi
		done
		echo "htmlserver end ok"
		;;
	kill)
		hs.do status
		killall -9 htmlserver
		;;
	restart)
		hs.do stop
		hs.do start
		;;
	restart_graceful)
		hs.do status
		PID=`ps -ef | grep -w htmlserver | grep -v grep | awk '{if($3=="1")print $2}'`
		if [ x"$PID" = x"" ] ; then
			echo "*** WARN : htmlserver not existed"
			exit 1
		fi
		kill -USR2 $PID
		while [ 1 ] ; do
			sleep 1
			PID2=`ps -ef | grep -w htmlserver | grep -v grep | awk -v pid="$PID" '{if($3=="1"&&$2!=pid)print $2}'`
			if [ x"$PID2" != x"" ] ; then
				break
			fi
		done
		echo "new htmlserver pid[$PID2] start ok"
		kill $PID
		while [ 1 ] ; do
			sleep 1
			PID3=`ps -ef | grep -w htmlserver | grep -v grep | awk -v pid="$PID" '{if($3=="1"&&$2==pid)print $2}'`
			if [ x"$PID3" = x"" ] ; then
				break
			fi
		done
		echo "old htmlserver pid[$PID] end ok"
		hs.do status
		;;
	relog)
		hs.do status
		PID=`ps -ef | grep -w htmlserver | grep -v grep | awk '{if($3=="1")print $2}'`
		if [ x"$PID" = x"" ] ; then
			echo "*** WARN : htmlserver not existed"
			exit 1
		fi
		kill -USR1 $PID
		echo "send signal to htmlserver for reopenning log"
		;;
	*)
		usage
		;;
esac

