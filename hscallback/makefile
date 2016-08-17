# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			HSCALLBACK_TEST.c \

###### 目标文件、安装目录配置区
include makeinstall
LIB		=	HSCALLBACK_TEST.so
LIBINST		=	$(_LIBINST)

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-I$(HOME)/include/htmlserver \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \
			-L$(HOME)/lib \
			-lhtmlserver \

###### 额外宏定义区
CLEAN_ADDITION	=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_LIBINST
#@ make_install_DFTHDERINST
#@ make_uninstall
#@ make_uninstall_LIBINST
#@ make_uninstall_DFTHDERINST
include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
HSCALLBACK_TEST.so		:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(SOFLAGS) $(LFLAGS)

