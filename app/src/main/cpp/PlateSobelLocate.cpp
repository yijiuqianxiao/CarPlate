///
//  PlateSobelLocate.cpp
//  MyCarPlate
//
//  Created by liuxiang on 2017/7/4.
//  Copyright © 2017年 liuxiang. All rights reserved.
//
#define TAG "sobelLocate"

#include "common.hpp"


char PlateSobelLocate::CHARS[]  = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','J','K','L','M','N','P','Q','R','S','T','U','V','W','X','Y','Z'};

string PlateSobelLocate::ZHCHARS[]  = {"川","鄂","赣","甘","贵","桂","黑","沪","冀","津","京","吉","辽","鲁","蒙","闽","宁","青","琼","陕","苏","晋","皖","湘","新","豫","渝","粤","云","藏","浙"};

PlateSobelLocate::PlateSobelLocate(const char* svmPath,const char* annPath,const char* annZhPath){
    svmClassifier = SVM::load(svmPath);
    svmClassifier->setKernel(SVM::KernelTypes::LINEAR);
    annClassifier = ANN_MLP::load(annPath);
    annZhClassifier = ANN_MLP::load(annZhPath);
    ann_size = Size(ANN_COLS,ANN_ROWS);
}

PlateSobelLocate::~PlateSobelLocate(){
    svmClassifier->clear();
    annClassifier->clear();
    annZhClassifier->clear();
}


//中国车牌的一般大小是440mm*140mm，面积为440*140，宽高比为3.14
//过滤掉一部分完全不可能是车牌的部分 就算不过滤也没关系 有svm分类
bool PlateSobelLocate::verifySizes(RotatedRect rrect) {
    //误差
    float error = .9f;
    //车牌规格 440×140 长宽比
    const float aspect =  3.14f;
    //最小区域
    int min = 20*aspect*20;
    //最大区域
    int max = 180*aspect*180;
    //考虑误差后的最小长宽比
    float rmin = aspect - aspect*error;
    //考虑误差后的最大长宽比
    float rmax = aspect + aspect*error;
    //区域面积
    int area = rrect.size.height * rrect.size.width;
    //区域宽高比
    float r = (float)rrect.size.width/(float)rrect.size.height;
    if(r <1)
        r = 1/r;
    //面积不能小于或者大于给到的区域 并且宽高比不能相差太大
    if( (area < min || area > max) || (r< rmin || r > rmax)  )
        return false;
    else
        return true;
}



void PlateSobelLocate::rotatedPlatMat(Mat &img_input, vector<RotatedRect> &vec_rotated_rects, vector <Mat>& vec_plate_mats ){
    for (int i = 0 ;i< vec_rotated_rects.size() ; ++i){
        //旋转区域
        RotatedRect rotatedRect = vec_rotated_rects[i];
        float angle = rotatedRect.angle;
        //如果宽比高小 则可能是竖的 需要再加90度
        float r = (float)rotatedRect.size.width / (float) rotatedRect.size.height;
        if(r<1)
            angle = 90 + angle;
        //获得旋转矩阵
        Mat mat_rotmat = getRotationMatrix2D(rotatedRect.center , angle,1);
        Mat img_rotated;
        //图像旋转
        warpAffine(img_input ,img_rotated,mat_rotmat, img_input.size(),INTER_CUBIC);
        
        //裁剪图像
        Size rect_size = rotatedRect.size;
        if(r<1)
            //变更宽高
            swap(rect_size.width, rect_size.height);
        Mat  img_crop;
        //截取特定区域
        getRectSubPix(img_rotated ,rect_size,rotatedRect.center , img_crop );
        
        
        //重定车牌大小为144x33 因为训练数据是144x33
        Mat img_resized(PLATE_ROW,PLATE_COLS,CV_8UC3);
        resize(img_crop , img_resized,img_resized.size() , 0,0,INTER_CUBIC);
        vec_plate_mats.push_back(img_resized);
        
        img_rotated.release();
        mat_rotmat.release();
        img_crop.release();
    }
}


void PlateSobelLocate::locate(Mat& img_src,vector<Mat>& out){


    Mat img_blur;
    //高斯模糊 模糊计算的半径影响后面的结果
    //去除一部分噪音
    GaussianBlur( img_src, img_blur, Size(5, 5),0);



    Mat img_gray;
    cvtColor(img_blur, img_gray, CV_RGB2GRAY);



    Mat grad_x;
    //边缘检测滤波器 边缘检测 便于区分车牌
    Sobel(img_gray, grad_x, CV_16S, 1, 0);

    Mat abs_grad_x;
    //绝对值,转换格式为8bit型
    convertScaleAbs(grad_x, abs_grad_x);

    Mat mat_threshold;
    threshold(abs_grad_x, mat_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);

    //闭操作 先膨胀、后腐蚀
    //把白色区域连接起来，或者扩大。任何黑色区域如果小于结构元素的大小都会被消除
    //对于结构大小 由于中国车牌比如 湘A 12345 有断层 所以 width过小不行,而过大会连接不必要的区域
    //sobel的方式定位不能100%匹配
    Mat element = getStructuringElement(MORPH_RECT, Size(17, 3));
    morphologyEx(mat_threshold, mat_threshold, MORPH_CLOSE, element);
    
       
       // IplImage qImg;
       // qImg = IplImage(mat_threshold);

       // cvSaveImage( "/sdcard/tess/1.jpg", &qImg);


    vector< vector<Point> > contours;
    findContours(mat_threshold,
                 contours,
                 CV_RETR_EXTERNAL,
                 CV_CHAIN_APPROX_NONE);
    
    vector<RotatedRect> vec_rect;
    
    vector< vector<Point> >::iterator itc = contours.begin();
   // int k = 0;
    while (itc != contours.end()) {
        RotatedRect mr = minAreaRect(Mat(*itc));

       // char filename[100];
       // sprintf(filename, "/sdcard/tess/train_%d.jpg",k);
       // IplImage qImg;
       // qImg = IplImage(img_src(boundingRect(*itc)));
       // k++;
       // cvSaveImage(filename, &qImg);

        if (verifySizes(mr)) {
            vec_rect.push_back(mr);
#ifdef MAC
            rectangle(img_src, Point(mr.center.x-mr.size.width/2,mr.center.y-mr.size.height/2),
                      Point(mr.center.x+mr.size.width/2,mr.center.y+mr.size.height/2), Scalar(0,255,255));
#endif
        }
        
        ++itc;
    }
    rotatedPlatMat(img_src,vec_rect,out);
    //TODO 再来一次轮廓发现 可以防止出现截取的区域过大 这里不再执行了
    //    int k = 0;
    for (vector<Mat>::iterator itc = out.begin(); itc != out.end(); ) {
        Mat img = *itc;
        //需要把候选车牌区域 图像中每个像素点作为一行特征向量，进行预测
        Mat img_gray;
        cvtColor(img, img_gray, CV_RGBA2GRAY);
        //灰度图象直方图均衡化 增强对比度 提高图像的质量
        equalizeHist(img_gray,img_gray);
        
        //        char filename[100];
        //        sprintf(filename, "resource/plate_%d.jpg",k);
        //        IplImage qImg;
        //        qImg = IplImage(img_gray); // cv::Mat -> IplImage
        //        cvSaveImage(filename, &qImg);
        //        k++;
        
        Mat p = img_gray.reshape(1,1);
        p.convertTo(p,CV_32FC1);
        
        int response = (int)svmClassifier->predict( p );
        p.release();
        img_gray.release();
        if (response){
            ++itc;
        }
        else{
            img.release();
            out.erase(itc);
        }
    }
    SHOW("sobelLocate", img_src);
    img_blur.release();
    img_gray.release();
    grad_x.release();
    abs_grad_x.release();
    mat_threshold.release();
}


//移除车牌固定点 固定点所在的行肯定色值上变化不大(基本上全是黑的) 所以如果出现这样的行 那么可以试着抹黑这一行
void PlateSobelLocate::clearFixPoint(Mat &img) {
    const int maxChange = 10;
    vector<int > vec;
    for (int i = 0; i < img.rows; i++) {
        int change = 0;
        for (int j = 0; j < img.cols - 1; j++) {
            //如果不一样则+1
            if (img.at<char>(i, j) != img.at<char>(i, j + 1)) change++;
        }
        vec.push_back(change);
    }
    for (int i = 0; i < img.rows; i++) {
        int change = vec[i];
        //跳跃数大于maxChange 的行数抹黑
        if (change <= maxChange) {
            for (int j = 0; j < img.cols; j++) {
                img.at<char>(i, j) = 0;
            }
        }
    }
    
}


void PlateSobelLocate::spatial_ostu(Mat mat, int w, int h) {
    
    //每次二值化一部分 能得到更好的效果
    int width = mat.cols / w;
    int height = mat.rows / h;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Mat src_cell = Mat(mat, Range(i * height, (i + 1) * height), Range(j * width, (j + 1) * width));
            threshold(src_cell, src_cell, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
        }
    }
}

//和verify车牌一样
bool PlateSobelLocate::verifyCharSizes(Mat r) {
    float aspect = 45.0f / 90.0f;
    float charAspect = (float)r.cols / (float)r.rows;
    float error = 0.7f;
    float minHeight = 10.f;
    float maxHeight = 35.f;
    float minAspect = 0.05f;
    float maxAspect = aspect + aspect * error;
    if (charAspect > minAspect && charAspect < maxAspect &&
        r.rows >= minHeight && r.rows < maxHeight)
        return true;
    else
        return false;
}

//得到中文后面的 轮廓下标
int PlateSobelLocate::getAfterChineseIndex( vector<Rect> vecRect) {
    int specIndex = 0;
    for (int i = 0; i < vecRect.size(); i++) {
        Rect mr = vecRect[i];
        int midx = mr.x + mr.width / 2;
        //车牌平均分为7份 如果当前下标对应的矩形右边大于1/7 且小于2/7
        if (midx < PLATE_COLS / 7 * 2 && midx > PLATE_COLS / 7) {
            specIndex = i;
        }
    }
    return specIndex;
}

Rect PlateSobelLocate::getChineseRect(Rect rectSpe) {
    int height = rectSpe.height;
    //宽度增加一点 数值是个大概就行 可以查看截出来的中文字符调整
    float width = rectSpe.width  * 1.15f;
    int x = rectSpe.x;
    int y = rectSpe.y;
    
    //x多减去一点点(中文和后面间隔稍微宽一点点)
    int newx = x - width * 1.15f;
    newx = newx > 0 ? newx : 0;
    
    Rect chineseRect(newx, y, width, height);
    
    return chineseRect;
}

string PlateSobelLocate::recognize(cv::Mat &plate){
    Mat mat_threshold;
    cvtColor(plate, mat_threshold, CV_BGR2GRAY);
    //    threshold(mat_threshold, mat_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
    //参数调整
    spatial_ostu(mat_threshold,3,3);
    clearFixPoint(mat_threshold);
    SHOW("clearLiuDing", mat_threshold);
    
    vector<vector<Point> > contours;
    findContours(mat_threshold,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    
    vector<vector<Point> >::iterator itc = contours.begin();
    vector<Rect> vecRect;
    while (itc != contours.end()) {
        Rect rect = boundingRect(Mat(*itc));
        Mat auxRoi(mat_threshold, rect);
        if (verifyCharSizes(auxRoi)){
            vecRect.push_back(rect);
#ifdef MAC
            rectangle(plate, Point(rect.x,rect.y), Point(rect.x+rect.width,rect.y+rect.height), Scalar(255,0,0));
#endif
        }
        ++itc;
    }
    SHOW("segment", plate);
    if (vecRect.empty()){
        return "";
    }
    //根据x坐标排序
    vector<Rect> sortedRect(vecRect);
    sort(sortedRect.begin(), sortedRect.end(),
         [](const Rect& r1, const Rect& r2) { return r1.x < r2.x; });
    
    int chineseIndex = getAfterChineseIndex(sortedRect);
    Rect chineseRect;
    if (chineseIndex < sortedRect.size())
        chineseRect = getChineseRect(sortedRect[chineseIndex]);
    else{
        LOGI("中文没截取到");
        return "";
    }
    
    //中文的矩形
    vector<Mat> vec_chars;
    Mat dst = mat_threshold(chineseRect);
    resize(dst,dst,ann_size);
    vec_chars.push_back(dst);
    
    
    
    //其他矩形
    int count = 6;
    for (int i = 0; i< sortedRect.size() && count ;++i){
        Rect rect = sortedRect[i];
        if (chineseRect.x+chineseRect.width > rect.x) {
            continue;
        }
        Mat dst = mat_threshold(rect);
        resize(dst,dst,ann_size);
        
        vec_chars.push_back(dst);
        --count;
    }
    
    
    
    //这里为了结构清晰一点 再遍历来识别
    string plate_result;
    int i = 0;
    for(;i<vec_chars.size();++i){
        Mat mat = vec_chars[i];
#ifdef MAC
        char a[10];
        sprintf(a, "car:%d",i);
        SHOW(a, mat);
#endif
        
//                char filename[100];
//                sprintf(filename, "resource/train_%d.jpg",i);
//                IplImage qImg;
//                qImg = IplImage(mat);
//                cvSaveImage(filename, &qImg);

        // char filename[100];
        // sprintf(filename, "/sdcard/tess/train_%d.jpg",i);
        // IplImage qImg(mat);
        // cvSaveImage(filename, &qImg);

        
        //结果
        Point maxLoc;
        
        Mat_<float> sampleMat = mat.reshape(1,1);
        Mat responseMat;
        if (i) {
            //预测
            annClassifier->predict(sampleMat,responseMat);
            //第三个参数是匹配度
            minMaxLoc(responseMat,0,0,0,&maxLoc);
            plate_result += CHARS[maxLoc.x];
        }else{
            //中文
            annZhClassifier->predict(sampleMat,responseMat);
            minMaxLoc(responseMat,0,0,0,&maxLoc);
            plate_result += ZHCHARS[maxLoc.x];
        }
        
        responseMat.release();
        sampleMat.release();
        mat.release();
        
    }
    CLEAR(vec_chars);
    
    return plate_result;
}
