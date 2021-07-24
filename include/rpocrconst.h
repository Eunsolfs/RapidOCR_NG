#pragma once

#ifdef WIN32
#define OS_SEP  "\\"
#else
#define OS_SEP "/"
#endif

#define RP_OCR_CLS_MODEL    "general_mobile_direction_cls.onnx"

#define RP_OCR_LANG_MODEL	 "general_language_classifier.onnx"

extern const char* G_OCR_DET_MODE[];

extern const char* RP_OCR_REC_MODEL[];
extern const char* RP_OCR_REC_DICT[];
extern  const char* RP_LANG_NAME[];
#define  MAX_LANG_SCOPE   QOLO_OTHER


const int	MAX_LANG_DET_BOX_NUMBER = 20;

