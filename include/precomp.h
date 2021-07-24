#pragma once

// dml  must be the first header 
#ifdef WIN32

#include "rpwinfunc.h"
#ifdef USE_DML

#include <onnxruntime/dml_provider_factory.h>

//#else
//#include <onnxruntime/cpu_provider_factory.h>
#endif

#ifdef USE_CUDA
#include <onnxruntime/cuda_provider_factory.h>
//#include <cuda_runtime.h>

#endif

#endif

#include <stdint.h>
#include <uchar.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <map>
#include <algorithm>

using namespace std;


#include "rpcommonfunc.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
using namespace cv;
#include <FreeImage.h>
#include "rpuniocr_api.h"
#include "rpocrdef.h"

#include "rpocrconst.h"


#include "onnxruntime/onnxruntime_run_options_config_keys.h"
#include "onnxruntime/onnxruntime_cxx_api.h"
// ocr
#include "rpexcept.h"

#include "clipper.h"
#include "utility.h"
#include "commonfunc.h"
#include "preprocess_op.h"
#include "postprocess_op.h"


#include "rpocrdet.h"
#include "rpocrcls.h"
#include "rpocrrec.h"
#include "rpbidi.h"
#include "rpuniocrobj.h"

