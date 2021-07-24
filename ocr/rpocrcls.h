#pragma once 

namespace RapidOCR {

class CRpOCRCls {
public:
  
   CRpOCRCls() = default;
   bool Init(const std::string &model_dir,
                      const int & num_thread,
                      const double &cls_thresh,
                        const bool use_gpu = false, int nGPUID=0
                      ) 
  {

    this->num_thread = num_thread;

    if(cls_thresh != 0.0f)
        this->cls_thresh = cls_thresh;

    LoadModel(model_dir, num_thread, use_gpu,nGPUID);
    m_bInit = true;
    return true;
  }

  // Load Paddle inference model
  void LoadModel(const std::string &model_dir, int nNumThread, bool bUseGPU=false, int nGPUID=0);
    cv::Mat Run(cv::Mat &img);

private:

  int num_thread = 4;
  bool use_mkldnn_ = false;
  double cls_thresh = 0.9;

  std::vector<int> cls_image_shape = { 3, 48, 192 };

  float  mean_[3] = { 0.5f, 0.5f, 0.5f };
  float scale_[3] = { 0.5f, 0.5f, 0.5f };
  bool is_scale_ = true;
  // pre-process
  ClsResizeImg resize_op_;
  Normalize normalize_op_;
  Permute permute_op_;

  bool m_bInit = false;
  string m_strInputName;
  string m_strOutputName;

  Ort::Session* m_session;
  Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "CRpOCRCls");
  Ort::SessionOptions sessionOptions = Ort::SessionOptions();


  string   m_strCPUGPU;
#ifdef USE_DML
  Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
#else
  Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  //Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

#endif

}; // class Classifier

} // namespace RapidOCR
