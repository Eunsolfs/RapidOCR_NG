// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

namespace RapidOCR {

class Normalize {
public:
    virtual  void  Run(cv::Mat& resize_img, std::vector<float> &inputTensorValues, const float* meanVals, const float* stdVals, const bool is_scale);
};

// RGB -> CHW
class Permute {
public:
  virtual void Run(const cv::Mat *im, float *data);
};

class DBNetResizeImg {
private:
    int m_max_side_len;
    int m_min_side_len;

    bool m_bLimitMax = false;
    int m_limit_side_len = 0;
public:
  virtual void Run(const cv::Mat &img, cv::Mat &resize_img, int max_side_len,int min_side_len, float &ratio_h, float &ratio_w);
  virtual void Run0(const cv::Mat& img, cv::Mat& resize_img, int max_side_len, int min_side_len, float& ratio_h, float& ratio_w);
  void Resize_Type0(const cv::Mat& img, cv::Mat& resize_img, float& ratio_h, float& ratio_w);
  void Resize_Type1(const cv::Mat& img, cv::Mat& resize_img, float& ratio_h, float& ratio_w);
  void Resize_Type2(const cv::Mat& img, cv::Mat& resize_img, float& ratio_h, float& ratio_w);

};

class CrnnResizeImg {
public:
  virtual void Run(const cv::Mat &img, cv::Mat &resize_img, float wh_ratio,
                    const std::vector<int> &rec_image_shape = {3, 32, 320});
};

class LangtypeResizeImg{
public:
    virtual void Run(const cv::Mat& img, cv::Mat& resize_img,
        const std::vector<int>& rec_image_shape = { 3, 32, 320 });
};


class ClsResizeImg {
public:
  virtual void Run(const cv::Mat &img, cv::Mat &resize_img,
                   const std::vector<int> &rec_image_shape = {3, 48, 192});
};


} // namespace RapidOCR