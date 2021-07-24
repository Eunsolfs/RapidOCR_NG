#include "precomp.h"


namespace RapidOCR
{


	void CRpOCRDet::LoadModel(const std::string& model_dir,int nNumThread, bool bUseGPU, int nGPUID)
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

	// Run predictor
	void CRpOCRDet::Run(cv::Mat& srcimg, std::vector<std::vector<std::vector<int>>>& boxes)
	{
		//std::vector<float> inputTensorValues = substractMeanNormalize(srcResize, mean_, normValues);
		float ratio_h{};
		float ratio_w{};

		cv::Mat resize_img;

		// resize: https://git-biz.qianxin-inc.cn/dlp/mousika/ocr/-/blob/master/QmNationOCR/python/general_mobile_v2_det/utils.py#L141
		this->resize_op_.Run(srcimg, resize_img, this->max_side_len_,this->min_side_len_, ratio_h, ratio_w);

		std::vector<float> inputTensorValues;
		normalize_op_.Run(resize_img, inputTensorValues, this->mean_, this->std_, this->is_scale_);


		std::array<int64_t, 4> inputShape{ 1, resize_img.channels(), resize_img.rows, resize_img.cols };
		Ort::Value inputTensor = Ort::Value::CreateTensor<float>(m_memoryInfo, inputTensorValues.data(),inputTensorValues.size(), inputShape.data(),inputShape.size());
		assert(inputTensor.IsTensor());
		const char* szInputName = m_strInputName.c_str();
		const char* szOutputName = m_strOutputName.c_str();


		Ort::RunOptions run_option;

		if (!m_strCPUGPU.empty())
			run_option.AddConfigEntry(kOrtRunOptionsConfigEnableMemoryArenaShrinkage, m_strCPUGPU.c_str());
		
		auto outputTensor = m_session->Run(run_option, &szInputName, &inputTensor, 1, &szOutputName, 1);

		assert(outputTensor.size() == 1 && outputTensor[0].IsTensor());
		std::vector<int64_t> outputShape = outputTensor[0].GetTensorTypeAndShapeInfo().GetShape();


		int64_t outputCount = std::accumulate(outputShape.begin(), outputShape.end(), 1,std::multiplies<int64_t>());
		float* floatArray = outputTensor[0].GetTensorMutableData<float>();
		std::vector<float> out_data(floatArray, floatArray + outputCount);


		// post process

		int n2 = outputShape[2];  //output_shape[2];
		int n3 = outputShape[3]; //output_shape[3];
		int n = n2 * n3;

		std::vector<float> pred(n, 0.0);
		std::vector<unsigned char> cbuf(n, ' ');

		for (int i = 0; i < n; i++) {
			pred[i] = float(out_data[i]);
			cbuf[i] = (unsigned char)((out_data[i]) * 255);  // for scale ,they are divided by 255, so need to restore it.
		}

		cv::Mat cbuf_map(n2, n3, CV_8UC1, (unsigned char*)cbuf.data());
		cv::Mat pred_map(n2, n3, CV_32F, (float*)pred.data());

		const double threshold = this->det_db_thresh_ * 255;
		const double maxvalue = 255;
		cv::Mat bit_map;
		cv::threshold(cbuf_map, bit_map, threshold, maxvalue, cv::THRESH_BINARY);
		cv::Mat dilation_map;
		cv::Mat dila_ele = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
		cv::dilate(bit_map, dilation_map, dila_ele);
		boxes = post_processor_.BoxesFromBitmap(
			pred_map, dilation_map, this->det_db_box_thresh_,
			this->det_db_unclip_ratio_, this->use_polygon_score_);

		boxes = post_processor_.FilterTagDetRes(boxes, ratio_h, ratio_w, srcimg);
	}
}