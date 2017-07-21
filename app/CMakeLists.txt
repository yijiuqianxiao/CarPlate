#CMake版本信息
cmake_minimum_required(VERSION 3.4.1)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#头文件配置
include_directories(src/main/cpp/include)

#编译源文件
file(GLOB my_source_path  src/main/cpp/*.cpp src/main/cpp/*.c)
add_library( carplate SHARED ${my_source_path} )

#添加 动态库 变量名为lib_opencv
add_library( lib_opencv SHARED IMPORTED)
#设置lib_opencv动态库的真实地址
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)

#查找系统日志库
find_library( log-lib log )
find_library(jnigraphics-lib
        jnigraphics )

target_link_libraries( carplate ${log-lib} ${jnigraphics-lib} lib_opencv)