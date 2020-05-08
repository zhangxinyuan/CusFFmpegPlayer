
cmake_minimum_required(VERSION 3.4.1)

# 1. 引入静态库的头文件，目的是将头文件目录追加到系统查找目录下，在C/C++文件中就可以通过 #ffmpeg.ffmpeg.include <xxx> 的方式来引入头文件
set(ffmpeg ${CMAKE_SOURCE_DIR}/ffmpeg)
include_directories(${ffmpeg}/include)

# 2. 设置预编译库所在路径。
#add_library() 这种方式加载动态库时有问题

# CMAKE_CXX_FLAGS 是系统变量，代表C++编译参数
# -L 指定库所在的目录(相对路径)
# 是追加到CMAKE_CXX_FLAGS下，不是覆盖
# 这里路径必须得写正确，否则会找不到实现
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../../../libs/${CMAKE_ANDROID_ARCH_ABI}")

file(GLOB SOURCE *.cpp)# 使用一个全局变量来指定cpp目录下所有的cpp源文件
add_library(
        native_player # 生成的库名
        SHARED    # 生成动态库；静态库使用STATIC
        ${SOURCE}) # 生成目标库的源代码

find_library(# 查找系统库(ndk下，编译在安卓系统中)，并生成一个变量名，用于在target_link_libraries中使用变量名
        log-lib # 库变量名
        log) # 库名

# 3. 链接预编译库
target_link_libraries(
        native_player
        avfilter avformat avcodec avutil swresample swscale
        ${log-lib}
        android
        z
        OpenSLES)