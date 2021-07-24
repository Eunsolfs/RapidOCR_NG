#include "precomp.h"

namespace RapidOCR
{ 
void CRpOCRCls::LoadModel(const std::string& model_dir, int nNumThread, bool bUseGPU, int nGPUID)
{

    if (bUseGPU)
    {
#ifdef USE_DML

        Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(sessionOptions, nGPUID));
        //else

        //   Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_CPU(sessionOptions, 1));
        //


        m_strCPUGPU = "";
#endif

#ifdef USE_CUDA


        OrtCUDAProviderOptions cuda_options{ 0 };
        cuda_options.device_id = 0;
        cuda_options.arena_extend_strategy = 0;
        cuda_options.gpu_mem_limit = 2 * 1024 * 1024 * 1024;//std::numeric_limits<size_t>::max(),
        cuda_options.cudnn_conv_algo_search = OrtCudnnConvAlgoSearch::EXHAUSTIVE;
        cuda_options.do_copy_in_default_stream = 1; // important
        cuda_options.has_user_compute_stream = 0;
        cuda_options.arena_extend_strategy = 0;

        sessionOptions.AppendExecutionProvider_CUDA(cuda_options);
        m_strCPUGPU = string_format("cpu:%d;gpu:%d", 0, 0);
#endif
}
    else
        m_strCPUGPU = string_format("cpu:%d", 0);


    sessionOptions.SetInterOpNumThreads(nNumThread);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);


#ifdef _WIN32
    std::wstring detPath = strToWstr(model_dir);
    m_session = new Ort::Session(env, detPath.c_str(), sessionOptions);
#else
    m_session = new Ort::Session(env, model_dir.c_str(), sessionOptions);
#endif




    getInputName(m_session, m_strInputName);
    getOutputName(m_session, m_strOutputName);


}

cv::Mat CRpOCRCls::Run(cv::Mat& img)
{

    cv::Mat resize_img;

    std::vector<int> cls_image_shape = { 3, 48, 192 };
    int index = 0;
    float wh_ratio = float(img.cols) / float(img.rows);

    this->resize_op_.Run(img, resize_img, cls_image_shape);

//#ifndef NDEBUG
//    cv::imwrite("cls_resized.png", resize_img);
//#endif

    std::vector<float> inputTensorValues;
    normalize_op_.Run(resize_img, inputTensorValues,this->mean_, this->scale_, this->is_scale_);

    std::array<int64_t, 4> inputShape{ 1, resize_img.channels(), resize_img.rows, resize_img.cols };

    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, inputTensorValues.data(),  inputTensorValues.size(), inputShape.data(), inputShape.size());
    assert(inputTensor.IsTensor());
    const char* szInputName = m_strInputName.c_str();
    const char* szOutputName = m_strOutputName.c_str();


    Ort::RunOptions run_option;
    if (!m_strCPUGPU.empty())
        run_option.AddConfigEntry(kOrtRunOptionsConfigEnableMemoryArenaShrinkage, m_strCPUGPU.c_str());



    auto outputTensor = m_session->Run(run_option, &szInputName, &inputTensor, 1, &szOutputName, 1);


    


    assert(outputTensor.size() == 1 && outputTensor.front().IsTensor());

    std::vector<int64_t> outputShape = outputTensor[0].GetTensorTypeAndShapeInfo().GetShape();

   
    int64_t outputCount = std::accumulate(outputShape.begin(), outputShape.end(), 1, std::multiplies<int64_t>());
    float* floatArray = outputTensor.front().GetTensorMutableData<float>();
    std::vector<float> softmax_out(floatArray, floatArray + outputCount);


    float score = 0;
    int label = 0;
    for (int i = 0; i < outputCount; i++) {
        if (softmax_out[i] > score) {
            score = softmax_out[i];
            label = i;
        }
    }
    if (label % 2 == 1 && score > this->cls_thresh) {
        cv::rotate(img, img, 1);
    }
    return img;

}



}