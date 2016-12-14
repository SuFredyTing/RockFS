SPDK_ROOT_DIR := /home/ystu/spdk
include ../mk/spdk.common.mk

######################################
#
#######################################
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
  
#target you can change test to what you want
#目标文件名，输入任意你想要的执行文件名
TARGET  := namei #table_stirage

#compile and lib parameter
#编译参数
CC      := gcc
LIBS    += 
LDFLAGS +=
DEFINES +=
INCLUDE += -I. 
CFLAGS  += -O3 $(DEFINES) $(INCLUDE) 
CXXFLAGS+= $(CFLAGS) -DHAVE_CONFIG_H
  
  
#i think you should do anything here
#下面的基本上不需要做任何改动了
.PHONY : everything objs clean veryclean rebuild
  
everything : $(TARGET)
  
all : $(TARGET)
  
objs : $(OBJS)
  
rebuild: veryclean everything
                
clean :
	rm -fr *.so
	rm -fr *.o
	rm -rf *.d
	rm -rf ../bin/*
 
veryclean : clean
	rm -fr $(TARGET)
  
$(TARGET) : $(OBJS)
	$(CC) `pkg-config fuse3 --cflags --libs` -o $@ $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) 
	mv $(TARGET) ../bin/

include ../mk/nvme.libtest.mk	
