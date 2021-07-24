#pragma once

namespace RapidOCR {

 typedef std::vector<std::vector<std::vector<int>>>  TXTBOX;

class CRpOCRDet {
public:

  CRpOCRDet() = default;
  bool Init(const std::string& model_dir,
      const int num_thread = 4,
      const int max_side_len = 960,
      const int min_side_len=200,
      const double det_db_thresh = 0.3,
      const double det_db_box_thresh = 0.5,
      const double det_db_unclip_ratio = 1.9,   // boxµÄ±ß´óÐ¡,default 2.0
      const bool use_polygon_score = true,
      const bool use_gpu = false, int nGPUID=0
                        ) {

    this->num_thread = num_thread;
    this->min_side_len_ = min_side_len;
    this->max_side_len_ = max_side_len;

    this->det_db_thresh_ = det_db_thresh;
    this->det_db_box_thresh_ = det_db_box_thresh;
    this->det_db_unclip_ratio_ = det_db_unclip_ratio;
    this->use_polygon_score_ = use_polygon_score;
   

    LoadModel(model_dir, num_thread, use_gpu ,nGPUID);
    m_bInit = true;
    return true;
  }

  // Load Paddle inference model
  void LoadModel(const std::string &model_dir, int nNumThread, bool bUseGPU=false, int nGPUID=0);

  // Run predictor
  void Run(cv::Mat &img, std::vector<std::vector<std::vector<int>>> &boxes);

private:

  bool m_bInit = false;
  
  
  int num_thread = 4;
  bool use_mkldnn_ = false;

  int max_side_len_ = 960;
  int min_side_len_ = 200;

  double det_db_thresh_ = 0.3;
  double det_db_box_thresh_ = 0.5;
  double det_db_unclip_ratio_ = 1.8;
  bool use_polygon_score_ = false;


  string m_strInputName;
  string m_strOutputName;

  float mean_[3] = {0.485f, 0.456f, 0.406f};
  float std_[3] = {0.229f, 0.224f, 0.225f};
  bool is_scale_ = true;  

  // pre-process
  DBNetResizeImg resize_op_;
  Normalize normalize_op_;
  Permute permute_op_;

  // post-process
  PostProcessor post_processor_;

  Ort::Session* m_session;
  Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "CRpOCRDet");
  Ort::SessionOptions sessionOptions = Ort::SessionOptions();

  string   m_strCPUGPU;

#ifdef USE_DML
  Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
#else
  Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  //Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
#endif
};

} // namespace RapidOCR