# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			IDL_htmlserver_conf.dsc.c \
			LOGC.c \
			rbtree.c \
			list.c \
			fasterjson.c \
			main.c \
			MonitorProcess.c \
			WorkerProcess.c \
			TimerThread.c \
			OnAcceptingSocket.c \
			OnReceivingSocket.c \
			OnSendingSocket.c \
			Envirment.c \
			Config.c \
			VirtualHostHash.c \
			HtmlCacheSession.c \
			HtmlCacheEventHander.c \
			HtmlCacheWdTree.c \
			HtmlCachePathfilenameTree.c \
			HttpSessionTimeoutTree.c \
			HttpSession.c \
			Util.c \
			MimeTypeHash.c \
			ProcessHttpRequest.c \

c_FILE2		=	\
			fasterhttp.c \

###### 目标文件、安装目录配置区
include makeinstall
BIN		=	htmlserver
BININST		=	$(_BININST)
LIB		=	libhtmlserver.so
LIBINST		=	$(_LIBINST)
HDER		=	fasterhttp.h \
			htmlserver.h
HDERINST	=	$(_HDERINST)

###### 编译选项
CFLAGS		=	$(_CFLAGS) \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \
			-L$(HOME)/lib \
			-lpthread \
			-lz \
			-lssl \

###### 额外宏定义区
CLEAN_ADDITION	=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_BININST
#@ make_install_LIBINST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST
#@ make_uninstall
#@ make_uninstall_BININST
#@ make_uninstall_LIBINST
#@ make_uninstall_HDERINST
#@ make_uninstall_DFTHDERINST
include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
htmlserver		:	$(c_FILE_o) $(c_FILE2_o)
	$(CC) -o $@ $(c_FILE_o) $(c_FILE2_o) $(LFLAGS)

libhtmlserver.so	:	$(c_FILE2_o)
	$(CC) -o $@ $(c_FILE2_o) $(SOFLAGS) $(LFLAGS)

