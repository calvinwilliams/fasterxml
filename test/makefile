# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			test_fasterxml.c \
			press_fasterxml.c \
			test_fasterhtml.c \

###### 目标文件、安装目录配置区
BIN		=	\
			test_fasterxml \
			press_fasterxml \
			test_fasterhtml \

###### 编译选项
CFLAGS		=	-I. \
			-I$(HOME)/include/fasterxml \

###### 链接选项
LFLAGS		=	-L. \
			-L$(HOME)/lib \
			-lfasterxml \

###### 额外宏定义区
CLEAN_ADDITION	=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_BININST
#@ make_install_LIBINST
#@ make_install_HDERINST

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
test_fasterxml		:	test_fasterxml.o
	$(CC) -o $@ test_fasterxml.o $(LFLAGS)

press_fasterxml		:	press_fasterxml.o
	$(CC) -o $@ press_fasterxml.o $(LFLAGS)

test_fasterhtml		:	test_fasterhtml.o
	$(CC) -o $@ test_fasterhtml.o $(LFLAGS)

