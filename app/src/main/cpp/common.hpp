//
//  common.h
//  opencvCar
//
//  Created by liuxiang on 2017/7/2.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#ifndef common_hpp
#define common_hpp


#include <opencv2/opencv.hpp>


#include <iostream>
using namespace std;
using namespace cv;
using namespace ml;

//xcode Preprocessor Macros 设置 MAC宏



//列向量 宽
#define ANN_COLS 8
//行向量  高
#define ANN_ROWS 16


#define PLATE_COLS 144
#define PLATE_ROW 33


#define CLEAR(vec) \
for(int i = 0;i < vec.size(); ++i){ \
     vec[i].release();   \
} \
vec.clear();

#ifdef MAC

//保存的SVM训练结果
#define SVM_XML "resource/SVM_DATA.xml"
//保存的ANN训练结果数据
#define ANN_XML "resource/ANN_DATA.xml"
#define ANN_ZH_XML "resource/ANN_ZH_DATA.xml"

#define LOGI(x) cout << TAG << " : " <<  x << endl
#define LOGE(x) cerr << TAG << " : " << x << endl

#define SHOW(name,mat) imshow(name, mat);


//向量
#define ANN_VEC ANN_COLS*ANN_ROWS

#include "AnnTrain.hpp"
#include "AnnTrainCH.hpp"
#include "SvmTrain.hpp"


#else



//保存的SVM训练结果
#define SVM_XML "/sdcard//SVM_DATA.xml"
//保存的ANN训练结果数据
#define ANN_XML "/sdcard/ANN_DATA.xml"
#define ANN_ZH_XML "/sdcard/ANN_ZH_DATA.xml"

#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>


#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#define SHOW(name,mat)



#endif

#include "PlateSobelLocate.hpp"




#endif /* common_hpp */
