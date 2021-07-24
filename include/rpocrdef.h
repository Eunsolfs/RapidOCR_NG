#pragma once




#define RP_ARRAY_SIZE(A)   sizeof(A)/sizeof(A[0])


struct TextBox {
    std::vector<cv::Point> boxPoint;
   
};


