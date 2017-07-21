//
//  main.cpp
//  opencvCar2
//
//  Created by liuxiang on 2017/7/4.
//  Copyright © 2017年 liuxiang. All rights reserved.
//
#define TAG "main"

#include "common.hpp"


//加载当前目录文件 使用相对路径 设置
//Product->Scheme->Edit Scheme->Working Dictionary->Using cusom working dictionary


/**
 * 定位车牌方法 这里实现的是sobel定位
 * sobel定位必须车牌区域能连接成一块白色区域 会出现无法连接成一块的情况,所以还可以加入其他定位方法一起定位增加识别率,如颜色定位
 *
 */

PlateSobelLocate *sobelLocate = 0;
#ifdef MAC
int main(int argc, const char * argv[]) {
    
    //    doAnnTrain();
    //    doAnnZhTrain();
    //    doSvmTrain();
    //载入图像
    Mat img_src = imread("resource/test3.jpg");
    sobelLocate = new PlateSobelLocate(SVM_XML,ANN_XML,ANN_ZH_XML);
#else
    
    void bitmap2Mat(JNIEnv *env, jobject bitmap, Mat *mat) {
        AndroidBitmapInfo info;
        void *pixels = 0;
        Mat &dst = *mat;
        //获得bitmap信息
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        //必须是 rgba8888 rgb565
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        //lock 获得数据
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        dst.create(info.height, info.width, CV_8UC3);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            LOGI("bitmap2Mat: RGBA_8888 bitmap -> Mat");
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
            cvtColor(dst, dst, COLOR_RGBA2RGB);
            tmp.release();
        } else {
            LOGI("bitmap2Mat: RGBA_565 bitmap -> Mat");
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            cvtColor(tmp, dst, COLOR_BGR5652RGB);
            tmp.release();
        }
        AndroidBitmap_unlockPixels(env, bitmap);
    }
    
    
    
    extern  "C"
    JNIEXPORT void JNICALL
    Java_com_dongnao_carplate_MainActivity_init(JNIEnv *env, jobject instance){
        if (sobelLocate)
            delete sobelLocate;
        sobelLocate = new PlateSobelLocate(SVM_XML,ANN_XML,ANN_ZH_XML);
    }
    
    extern  "C"
    JNIEXPORT void JNICALL
    Java_com_dongnao_carplate_MainActivity_release(JNIEnv *env, jobject instance){
        if (sobelLocate)
            delete sobelLocate;
    }
    
    
    extern  "C"
    JNIEXPORT jstring JNICALL
    Java_com_dongnao_carplate_MainActivity_recognition(JNIEnv *env, jobject instance, jobject bitmap){
        Mat img_src;
        bitmap2Mat(env, bitmap, &img_src);
        
#endif
        resize(img_src,img_src,Size(640,480));
        
        vector<Mat> plates;
        
        sobelLocate->locate(img_src,plates);
        if(plates.empty())
        {
            LOGE("not found car plate!");
#ifdef MAC
            waitKey(0);
            delete sobelLocate;
            img_src.release();
            
            return 0;
#else
            img_src.release();
            return env->NewStringUTF("");
#endif
        }
        SHOW("carPlate", plates[0]);
        string result = sobelLocate->recognize(plates[0]);
#ifdef MAC
        LOGI(result);
        waitKey();
#endif
        CLEAR(plates);
        img_src.release();
        
#ifdef MAC
        delete sobelLocate;
        return 1;
#else
        LOGI("%s",result.c_str());
        return env->NewStringUTF(result.c_str());
#endif
    }
