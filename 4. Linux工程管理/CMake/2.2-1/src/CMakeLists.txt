# 单个目录实现
# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 工程
# PROJECT(0VOICE)
# 手动加入文件
SET(SRC_LIST main.c)
MESSAGE(STATUS "THIS IS BINARY DIR " ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "THIS IS SOURCE DIR " ${PROJECT_SOURCE_DIR})

ADD_EXECUTABLE(youzi ${SRC_LIST})
# 演示
ADD_EXECUTABLE(youzi2 ${SRC_LIST})
# 将执行文件安装到bin目录
# 默认/usr/local/
#指定自定义目录，比如 cmake -DCMAKE_INSTALL_PREFIX=/tmp/usr ..
INSTALL(TARGETS youzi RUNTIME DESTINATION bin)
INSTALL(TARGETS youzi2 RUNTIME DESTINATION bin)