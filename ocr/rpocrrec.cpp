#include "precomp.h"
namespace RapidOCR
{

	void CRpOCRRec::LoadModel(const std::string& model_dir, int nNumThread, bool bUseGPU, int nGPUID)
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
    void CRpOCRRec::Run(std::vector<std::vector<std::vector<int>>> boxes, cv::Mat& img, CRpOCRCls* cls,  RAPID_OCR_RESULT& Result)
	{
        cv::Mat crop_img;
        cv::Mat resize_img;
        cv::Mat srcimg;
        img.copyTo(srcimg);
   //#ifndef NDEBUG

   //     stringstream ssTest;
   //#endif  


        Result.Result.clear();
        
        if (boxes.size() > 0)
            Result.bResult = true;
        else
            Result.bResult = false;

      
      
        int nMaxHeight=0, nMaxWidth=0;  // rows, cols

        vector<cv::Mat> AllImages;
        for (auto & singleBox: boxes)
        {
            crop_img = Utility::GetRotateCropImage(srcimg, singleBox);


            //#ifndef NDEBUG
            //            ssTest.str("");
            //            ssTest << "test_box_" << i << ".png";
            //            cv::imwrite(ssTest.str(), crop_img);
            //#endif 

            if (cls != nullptr) {
                crop_img = cls->Run(crop_img);
            }
       

            float wh_ratio = float(crop_img.cols) / float(crop_img.rows);

            this->resize_op_.Run(crop_img, resize_img, wh_ratio);

            AllImages.push_back(resize_img);
        
           
        }

      

     
        for (int i = 0; i < AllImages.size(); i += m_nBatchSize)
        {
            int bBatchSizePer = (AllImages.size() - i) >= m_nBatchSize ? m_nBatchSize : ( AllImages.size() - i);

            std::vector<float> inputTensorValues;
            for (int s = 0; s < bBatchSizePer; s++)
            {
                auto SingleImage = AllImages[i + s];
                if (SingleImage.rows > nMaxHeight)
                    nMaxHeight = SingleImage.rows;

                if (SingleImage.cols > nMaxWidth)
                    nMaxWidth = SingleImage.cols;
            }

            for (int s = 0; s < bBatchSizePer; s++)
            {
                cv::Mat unifiedImge;
                auto SingleImage = AllImages[i+ s];
                cv::copyMakeBorder(SingleImage, unifiedImge, 0, 0, 0, nMaxWidth - SingleImage.cols, cv::BORDER_CONSTANT, { 127, 127, 127 });


               // imwrite("test.png", unifiedImge);

                normalize_op_.Run(unifiedImge, inputTensorValues, mean_, scale_, true);
            }
            // inreference 


            std::array<int64_t, 4> inputShape{ bBatchSizePer, resize_img.channels(), nMaxHeight, nMaxWidth };
            Ort::Value inputTensor = Ort::Value::CreateTensor<float>(m_memoryInfo, inputTensorValues.data(),  inputTensorValues.size(), inputShape.data(), inputShape.size());
            assert(inputTensor.IsTensor());
            const char* szInputName = m_strInputName.c_str();
            const char* szOutputName = m_strOutputName.c_str();

            Ort::RunOptions run_option;

            if (!m_strCPUGPU.empty())
                run_option.AddConfigEntry(kOrtRunOptionsConfigEnableMemoryArenaShrinkage, m_strCPUGPU.c_str());


            // run
            auto outputTensor = m_session->Run(run_option, &szInputName, &inputTensor, 1, &szOutputName, 1);

            
            assert(outputTensor.size() == 1 && outputTensor[0].IsTensor());

  /*          struct OrtTensorTypeAndShapeInfo* shape_info;
            ORT_ABORT_ON_ERROR(g_ort->GetTensorTypeAndShape(outputTensor, &shape_info));
            size_t dim_count;
            ORT_ABORT_ON_ERROR(g_ort->GetDimensionsCount(shape_info, &dim_count));*/

        
          

           
            std::vector<int64_t> outputShape = outputTensor[0].GetTensorTypeAndShapeInfo().GetShape();
            auto type_info = outputTensor[0].GetTensorTypeAndShapeInfo();
            auto dim_y = type_info.GetShape();
            size_t total_len = type_info.GetElementCount();

            for (int i = 0; i < outputShape[0]; i++)
            {
                // ASSERT_EQ(values_y.size(), total_len);
                
                 int64_t out_num = std::accumulate(outputShape.begin()+1, outputShape.end(), 1, std::multiplies<int64_t>());

                 float* floatArray = outputTensor[0].GetTensorMutableData<float>();
                 std::vector<float> predict_batch(floatArray+(i* out_num), floatArray + (i * out_num) + out_num);

          
                 // ctc decode
                 std::vector<std::string> str_res;
                 int argmax_idx;
                 int last_index = 0;
                 float score = 0.f;
                 int count = 0;
                 float max_value = 0.0f;

                 // outshape[1]: h
                 //outshape[2] : w

                 uint32_t h = outputShape[1];
                 uint32_t w = outputShape[2];

                 for (int n = 0; n < h; n++) {
                     argmax_idx = int(Utility::argmax(&predict_batch[n * w], &predict_batch[(n + 1) * w-1]));
                     max_value =  float(*std::max_element(&predict_batch[n * w], &predict_batch[(n + 1) * w-1]));

                     if (argmax_idx > 0 && (!(n > 0 && argmax_idx == last_index))) {
                         score += max_value;
                         count += 1;
                         str_res.push_back(label_list_[argmax_idx]);
                     }
                     last_index = argmax_idx;
                 }
                 score /= count;
                 string strRetTxt;
                 for (int i = 0; i < str_res.size(); i++) {
                     strRetTxt+= str_res[i];
                 }
                 RP_OCR_RESULT_ITEM Item{};
                 Item.strTxt = strRetTxt;
                 Item.score = score;
                 Item.Box = boxes[i]; //return the box

                 Result.Result.push_back(Item);

                 
            }
           // std::cout << strRetTxt<<"\tscore: " << score << std::endl;
            

        }

  	}



}