CROSS =
# 定义CC为gcc编译
CC = $(CROSS)gcc  
# 定义CXX为g++编译    
CXX = $(CROSS)g++     
# 定义DEBUG 方式为 -g -O2
DEBUG = -g -O2        
CFLAGS = $(DEBUG) -Wall -c
RM = rm -rf

# /定义SRC为当前工程目录下所有的.cpp文件
SRCS = $(wildcard ./*.c)   
# 定义OBJS为SRCS对应的.o文件
OBJS = $(patsubst %.c, %.o, $(SRCS))    
# 定义HEADER_PATH为当前工程中的头文件路径
HEADER_PATH = -I ./include/   
# 定义LIB_PATH为当前工程中的头文件路径 
LIB_PATH = -L ./lib/        
# 输出当前LIB_PATH中的内容   
$(warning LIB_PATH)         
# 制定LIBS链接库的名称 
LIBS=-lpthread                 
# lib中的库文件名称为libpthread.so

# 定义当前生成的版本
VERSION = 1.0.0        
# 定义生成可执行文件的名称       
TARGET = simple.$(VERSION)       

$(TARGET) : $(OBJS)
# 告诉编译器生成可执行文件时库存放的目录，以及库的名字
	$(CXX) $^ -o $@ $(LIB_PATH) $(LIBS)    
$(OBJS):%.o : %.c
#告诉编译器申城中间文件时头文件的所在目录
	$(CXX) $(CFLAGS) $< -o $@ $(HEADER_PATH)  
clean:  
	$(RM) $(TARGET) *.o 
