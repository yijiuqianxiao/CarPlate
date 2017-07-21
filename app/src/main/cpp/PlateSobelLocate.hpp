//
//  PlateSobelLocate.hpp
//  MyCarPlate
//
//  Created by liuxiang on 2017/7/4.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#ifndef PlateSobelLocate_hpp
#define PlateSobelLocate_hpp

//按照sobel查找

class PlateSobelLocate{
    
public:
    PlateSobelLocate(const char* svmPath,const char* annPath,const char* annZhPath);
    ~PlateSobelLocate();
    
    //定位车牌
    void locate(Mat& img_src,vector<Mat>& out);
    
    //进行识别
    string recognize(Mat& plate);
    
private:
    //粗略判断是否满足车牌条件
    bool verifySizes(RotatedRect rrect);
    //获得车牌区域
    void rotatedPlatMat(Mat &img_input, vector<RotatedRect> &vec_rotated_rects, vector <Mat>& vec_plate_mats );
    //移除车牌固定的点
    void clearFixPoint(Mat &img);
    //按区域二值化
    void spatial_ostu(Mat mat, int w, int h);
    //判断是否是车牌字符
    bool verifyCharSizes(Mat r);
    //获得中文后面的字符下标 (因为中文可能被截断 比较特殊)
    int getAfterChineseIndex( vector<Rect> vecRect);
    //获得中文的rect
    Rect getChineseRect( Rect rectSpe);
private:
    Ptr<SVM>  svmClassifier;
    Ptr<ANN_MLP> annClassifier;
    Ptr<ANN_MLP> annZhClassifier;
    static char CHARS[];
    static string ZHCHARS[];
    Size ann_size;
};

#endif /* PlateSobelLocate_hpp */
