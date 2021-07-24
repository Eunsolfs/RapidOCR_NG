#include "precomp.h"


const char * G_OCR_DET_MODE[] = { "general_mobile_v2_det.onnx" ,"general_server_v2_det.onnx" };

// QOLO_CH_ENG = 1,
//QOLO_KOREAN = 2,
//QOLO_MONGOLIA = 3,
//QOLO_UYGUR = 4,
//QOLO_TIBET = 5,
const char* RP_OCR_REC_MODEL[] = {"mobile_ChEng_v2_rec.onnx","mobile_hw_v2_rec.onnx","mobile_korean_v2_rec.onnx","mobile_mongolian_v2_rec.onnx","mobile_uighur_v2_rec.onnx","mobile_tibetan_v2_rec.onnx"};
const char* RP_OCR_REC_DICT[] = { "chinese_keys.txt","chinese_keys.txt","korean_keys.txt","mongolian_keys.txt","uighur_keys.txt","tibetan_keys.txt" };


const char* RP_LANG_NAME[] = { "Chinese_English","Handwriting","Korean","Mongolian","Uighur","Tibetan","Other" };