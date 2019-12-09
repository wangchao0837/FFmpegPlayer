
cmake_minimum_required(VERSION 3.4.1)

## CMAKE_SOURCE_DIR：表示cmakeList所在路径
## 添加cmakeList所在路径/codec/ 下所有的cpp源文件编译
file(GLOB source ${CMAKE_SOURCE_DIR}/codec/*.cpp)
add_library(
        ffmpeg-native
        SHARED
        ${source}
        native-lib.cpp
        )

##配置 预编译库 如ffmpeg库 路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_GLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")

##加入预编译库的头文件 如ffmpeg库 的头文件
include_directories(include)

#添加链接库 链接到ffmpeg-native中
target_link_libraries(
        ffmpeg-native
        avfilter
        avformat
        avcodec
        avutil
        swresample
        swscale
        z
        android
        OpenSLES
        log)