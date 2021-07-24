#pragma once


namespace RapidOCR {


    typedef std::vector<std::vector<int>> SINGLEBOX;
typedef struct _qm_ocr_result
{

    string strTxt;
    float score;
    SINGLEBOX Box;
    int nRCenter;
}RP_OCR_RESULT_ITEM;

typedef struct __rapid_ocr_result
{
    vector<RP_OCR_RESULT_ITEM> Result;
    bool			bResult; // true/false
    RP_OCR_LANG_TYPE  Lang;
    void* pObj;
   
}RAPID_OCR_RESULT;


class CRpOCRRec {
public:

   CRpOCRRec() = default;

  bool IsSetup() { return m_bInit; }
  bool Init(const std::string &model_dir,
                          const int num_thread,
                          const string &label_path, RP_OCR_LANG_TYPE Lang,bool bUseGPU, int nGPUID, int nBatchSize = 6) {

    this->num_thread = num_thread;

    m_LangType = Lang;
    this->label_list_ = Utility::ReadDict(label_path);
    this->label_list_.insert(this->label_list_.begin(),"#"); // blank char for ctc
    this->label_list_.push_back(" ");

    LoadModel(model_dir, num_thread, bUseGPU,nGPUID);
    m_bInit = true;
    m_nBatchSize = nBatchSize;
    return true;
  }

  // Load onnx inference model
  void LoadModel(const std::string &model_dir, int nNumThread, bool bUseGPU=false, int nGPUID=0);

void  Run(std::vector<std::vector<std::vector<int>>> boxes, cv::Mat &img,CRpOCRCls*cls,RAPID_OCR_RESULT& Result);


private:

  bool m_bInit = false;
  int num_thread = 4;
  int m_nBatchSize = 6;

  RP_OCR_LANG_TYPE  m_LangType;
  std::vector<std::string> label_list_;

  float mean_[3] = { 0.5f, 0.5f, 0.5f };
  float scale_[3] = { 0.5f, 0.5f, 0.5f };

  bool is_scale_ = true;

  // pre-process
  CrnnResizeImg resize_op_;
  Normalize normalize_op_;
  //Permute permute_op_;


  string m_strInputName;
  string m_strOutputName;

  // post-process
  PostProcessor post_processor_;
  Ort::Session* m_session;
  Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "CRpOCRRec");
  Ort::SessionOptions sessionOptions = Ort::SessionOptions();

  string   m_strCPUGPU;

#ifdef USE_DML
    Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault); 
#else
  Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  //Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
#endif


}; // class CRpOCRRec

} // namespace RapidOCR
