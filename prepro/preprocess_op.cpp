#include "precomp.h"

namespace RapidOCR {

void Permute::Run(const cv::Mat *im, float *data) {
  int rh = im->rows;
  int rw = im->cols;
  int rc = im->channels();
  for (int i = 0; i < rc; ++i) {
    cv::extractChannel(*im, cv::Mat(rh, rw, CV_32FC1, data + i * rh * rw), i);
  }
}

 void Normalize::Run(cv::Mat & resize_img, std::vector<float> &inputTensorValues,const float* meanVals, const float* stdVals, const bool is_scale)
{
    if (is_scale)
        resize_img.convertTo(resize_img, CV_32F, 1.0 / 255);  //divided by 255, scale

    cv::Mat channels[3]; //借用来进行HWC->CHW
    cv::split(resize_img, channels);
    for (int i = 0; i < resize_img.channels(); i++)
    {

        channels[i] -= meanVals[i];  // mean
        if(stdVals[i] != 1.0f)
            channels[i] /= stdVals[i];   // std
    }
    for (int i = 0; i < resize_img.channels(); i++)  // BGR2RGB, HWC->CHW
    {
        std::vector<float> data = std::vector<float>(channels[2 - i].reshape(1, resize_img.cols * resize_img.rows));
        inputTensorValues.insert(inputTensorValues.end(), data.begin(), data.end());
    }

}

 //https://git-biz.qianxin-inc.cn/dlp/mousika/ocr/-/blob/master/QmNationOCR/python/general_mobile_v2_det/utils.py#L141


void DBNetResizeImg::Run(const cv::Mat &img, cv::Mat &resize_img,int max_side_len, int min_side_len, float &ratio_h, float &ratio_w) {

    m_max_side_len = max_side_len;
    m_min_side_len = min_side_len;


  int w = img.cols;
  int h = img.rows;




  if (h < 200 || (h > 200 && w >= 5000))
  {

      m_bLimitMax = true;
      m_limit_side_len=5000;
  }


  if (h > 200 && w < 5000)
  {
      m_bLimitMax = false;
      m_limit_side_len= 736;
  }



  Resize_Type0(img, resize_img, ratio_h, ratio_w);


  //float ratio = 1.f;
  //int max_wh = w >= h ? w : h;
  //if (max_wh > max_side_len) {
  //  if (h > w) {
  //    ratio = float(max_side_len) / float(h);
  //  } else {
  //    ratio = float(max_side_len) / float(w);
  //  }
  //}

  //int resize_h = int(float(h) * ratio);
  //int resize_w = int(float(w) * ratio);

  //resize_h = max(int(round(float(resize_h) / 32) * 32), 32);
  //resize_w = max(int(round(float(resize_w) / 32) * 32), 32);

  //cv::resize(img, resize_img, cv::Size(resize_w, resize_h));
  //ratio_h = float(resize_h) / float(h);
  //ratio_w = float(resize_w) / float(w);

  // by default, we use auto mode.


}

void DBNetResizeImg::Run0(const cv::Mat& img, cv::Mat& resize_img, int max_side_len, int min_side_len, float& ratio_h, float& ratio_w) {

    m_max_side_len = max_side_len;
    m_min_side_len = min_side_len;


    int w = img.cols;
    int h = img.rows;


    float ratio = 1.f;
    int max_wh = w >= h ? w : h;
    if (max_wh > max_side_len) {
      if (h > w) {
        ratio = float(max_side_len) / float(h);
      } else {
        ratio = float(max_side_len) / float(w);
      }
    }

    int resize_h = int(float(h) * ratio);
    int resize_w = int(float(w) * ratio);

    resize_h = max(int(round(float(resize_h) / 32) * 32), 32);
    resize_w = max(int(round(float(resize_w) / 32) * 32), 32);

    cv::resize(img, resize_img, cv::Size(resize_w, resize_h));
    ratio_h = float(resize_h) / float(h);
    ratio_w = float(resize_w) / float(w);

    // by default, we use auto mode.


}

void DBNetResizeImg::Resize_Type0(const cv::Mat& img, cv::Mat& resize_img, float& ratio_h, float& ratio_w)
{

    int w = img.cols;
    int h = img.rows;

    float ratio = 1.f;

    if (m_bLimitMax) //max
    {
        if (max(h, w) > m_limit_side_len)
        {
            if (h > w)
                ratio = float(m_limit_side_len) / h;
            else
                ratio = float(m_limit_side_len) / w;
        }
  
    }
    else
    {
        if (min(h, w) < m_limit_side_len)
        {

            if (h < w)
                ratio = float(m_limit_side_len) / h;
            else
                ratio = float(m_limit_side_len) / w;
        }


    }

    int resize_h = int(h * ratio);
    int resize_w = int(w * ratio);

    resize_h = int(round(float(resize_h) / 32) * 32);
    resize_w = int(round(float(resize_w) / 32) * 32);


    cv::resize(img, resize_img, cv::Size(resize_w, resize_h));

    ratio_h = float(resize_h) / float(h);
    ratio_w = float(resize_w) / float(w);
}
void DBNetResizeImg::Resize_Type1(const cv::Mat& img, cv::Mat& resize_img, float& ratio_h, float& ratio_w)
{

}
void DBNetResizeImg::Resize_Type2(const cv::Mat& img, cv::Mat& resize_img, float& ratio_h, float& ratio_w)
{


}


void CrnnResizeImg::Run(const cv::Mat &img, cv::Mat &resize_img, float wh_ratio,
                        const std::vector<int> &rec_image_shape) {
  int imgC, imgH, imgW;
  imgC = rec_image_shape[0];
  imgH = rec_image_shape[1];
  //imgW = rec_image_shape[2];

  imgW = int(32 * wh_ratio);

  float ratio = float(img.cols) / float(img.rows);
  int resize_w;
  if (ceilf(imgH * ratio) > imgW)
    resize_w = imgW;
  else
    resize_w = int(ceilf(imgH * ratio));
  {
    cv::resize(img, resize_img, cv::Size(resize_w, imgH), 0.f, 0.f, cv::INTER_LINEAR);
    cv::copyMakeBorder(resize_img, resize_img, 0, 0, 0, int(imgW - resize_img.cols), cv::BORDER_CONSTANT, {127, 127, 127});
  } 
}

void LangtypeResizeImg::Run(const cv::Mat& img, cv::Mat& resize_img,
    const std::vector<int>& rec_image_shape) {
    int imgC, imgH, imgW;
    imgC = rec_image_shape[0];
    imgH = rec_image_shape[1];
    imgW = rec_image_shape[2];

    
    cv::Mat image_temp;
    int h = img.rows;
    int w = img.cols;
   
    cv::resize(img, image_temp, cv::Size(int(32.0f / h * w), 32), 0.f, 0.f, cv::INTER_LINEAR);
    int h_temp = image_temp.rows, w_temp = image_temp.cols;

    if (w / h < 320 / 32)
    {
    
        int ttt = int((320 - w_temp) / 2);
        cv::copyMakeBorder(image_temp, resize_img, 0.f, 0.f, ttt, 320 - w_temp - ttt, cv::BORDER_CONSTANT, { 0, 0, 0 });
    }
    if (w / h > 320 / 32)
        // 如果太长，就截取其中的一部分
    {
        int ttt = int((w_temp - 320) / 2);


        Rect rect(ttt,0, 320, 32);
        resize_img = image_temp(rect);
    }
    if (w / h == 320 / 32)
    {
        cv::resize(image_temp, resize_img, cv::Size(320, 32), 0.f, 0.f, cv::INTER_LINEAR);
    }

}
void ClsResizeImg::Run(const cv::Mat &img, cv::Mat &resize_img,
                        const std::vector<int> &rec_image_shape) {
  int imgC, imgH, imgW;
  imgC = rec_image_shape[0];
  imgH = rec_image_shape[1];
  imgW = rec_image_shape[2];


  /*
  float ratio = float(img.cols) / float(img.rows);
  int resize_w, resize_h;
  if (ceilf(imgH * ratio) > imgW)
    resize_w = imgW;
  else
    resize_w = int(ceilf(imgH * ratio));

   {
    cv::resize(img, resize_img, cv::Size(resize_w, imgH), 0.f, 0.f,
               cv::INTER_LINEAR);
    if (resize_w < imgW) {
      cv::copyMakeBorder(resize_img, resize_img, 0, 0, 0, imgW - resize_w,
                         cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    }
  } 

  */

  int h = img.rows;
  int w = img.cols;
  int new_width;
  if (w / h > 6.4)
  {
      //# 按照训练集图像来截取
      new_width = (int)(6.4 * h);
      Rect rect(0, 0, new_width, h);
      resize_img = img(rect);

  }
  else
      resize_img = img;
  float ratio = w / float(h);
  int resized_w;
  if (ceil(imgH * ratio) > imgW)
      resized_w = imgW;
  else
  {
      resized_w = int(ceil(imgH * ratio));
  }
  cv::resize(resize_img, resize_img, Size(resized_w, imgH));
}

} // namespace RapidOCR
