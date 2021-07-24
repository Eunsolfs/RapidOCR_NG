#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif 

#ifdef WIN32

#ifdef _RPUNIOCR_EXPORT_LIB
#define _RP_OCR_API  __declspec(dllexport)
#else
#define _RP_OCR_API __declspec(dllimport)
#endif
#else

#define _RP_OCR_API 
#endif

	typedef void* RPHANDLE;
#ifndef _USE_FREEIMAGE
	typedef  char RP_BOOL;
#endif

	typedef void* RPRETHANDLE;

#ifndef NULL
#define NULL 0
#endif 
#define	TRUE	1
#define	FALSE	0

	typedef enum __qmuni_ocr_pipeline
	{
		QOP_NONE		= 0,
		QOP_OCR			= 1,
		QOP_IMG_LANG	= 2,
		QOP_OCR_ALL		= 0XFF,

	}RPOCR_PIPELINE;

	typedef enum qm_ocr_lang
	{
		QOL_ALL			= 0XFF,
		QOL_CH_ENG		= 1,
		QOL_HW_CH_ENG	= 2,
		QOL_KOREAN		= 4,
		QOL_MONGOLIA	= 8,
		QOL_UYGUR		= 16,
		QOL_TIBET		= 32,
		QOL_OTHER		= QOL_CH_ENG,
	}RP_OCR_LANG_MASK;

	typedef enum qm_ocr_lang_offset
	{
		QOLO_NONE = -1,
		QOLO_ALL = 0xFF,
		QOLO_CH_ENG = 0,
		QOLO_HW_CH_ENG=1,
		QOLO_KOREAN = 2,
		QOLO_MONGOLIA =3,
		QOLO_UYGUR = 4,
		QOLO_TIBET = 5,
		QOLO_OTHER = 6,

	}RP_OCR_LANG_OFFSET;

	typedef RP_OCR_LANG_OFFSET RP_OCR_LANG_TYPE;

	// for real language-type offset, need subtract 1 from them.

	typedef enum
	{
		QOMT_RAPID = 0,
		QOMT_FINE = 1,

	}RP_OCR_MODEL_TYPE;

	typedef struct qm_ocr_param
	{
		int		num_thread;
		int		max_side_len;
		int		min_side_len;
		float	det_db_thresh;
		float	det_db_box_thresh;
		float	det_db_unclip_ratio;		// box的边大小,default 2.0
		int		use_polygon_score;
		float	cls_thresh;				// 方向检查阈值 
		int		strength_degree;			//default 0 // 对比度增强，数值 0-n
	

	}RPOCR_PARAM;


	typedef struct qm_ocr_init_info
	{

		const char*			szDetModelDir;
		RP_OCR_LANG_MASK	Lang;
		RP_BOOL				bEncrypted;
		int					nThreads;  // the number of threads.
		RP_OCR_MODEL_TYPE	OcrMode;
		RPOCR_PIPELINE		PipLine;
		RPOCR_PARAM			OcrParam;
		RP_OCR_LANG_TYPE	DefaultLang;  //to aim to default language
		RP_BOOL				bForceCPU;
		RP_BOOL				bForceGPU;
		int					nGUPID;
		int					nBatchSize;
		int					bDebugInfo;					// default 0 不显示；1 显示debug info
	}RP_OCR_INIT_INFO;



#ifdef OLDCRC
#define POLY64REV	0xd800000000000000ULL
#define INITIALCRC	0x0000000000000000ULL
#else
#define POLY64REV     0x95AC9329AC4BC9B5ULL
#define INITIALCRC    0xFFFFFFFFFFFFFFFFULL
#endif

	

	typedef struct
	{
		int Left;
		int Right;
		int Top;
		int Bottom;
		const char* szString;

	}RPOCR_DET_BLOCK;

	
	_RP_OCR_API void   RPOcrPrepare(RP_OCR_INIT_INFO* InitInfo);
	_RP_OCR_API RPHANDLE   RPOcrInit(RP_OCR_INIT_INFO * InitInfo);
	
	_RP_OCR_API   RPRETHANDLE RPOcrDoOcr(RPHANDLE handle, const char* szImgPath);
	_RP_OCR_API   RPRETHANDLE RPOcrDoOcrMem(RPHANDLE handle, const char* pImgBuf, int nLen);

	_RP_OCR_API  int RPOcrGetLen(RPRETHANDLE handle);

	_RP_OCR_API  int RPOcrGetResult(RPRETHANDLE handle, char* szBuf, int nLen, float nScore);

	_RP_OCR_API  RP_OCR_LANG_TYPE RPOcrGetLang(RPRETHANDLE handle);

	_RP_OCR_API  int RPOCrGetBlockNum(RPRETHANDLE handle);

	_RP_OCR_API  RPOCR_DET_BLOCK RPOcrGetBlock(RPRETHANDLE handle,int nIndex);


	_RP_OCR_API  void RPOcrFree(RPRETHANDLE handle);
	_RP_OCR_API void  RPOcrDeinit(RPHANDLE handle);




#ifdef __cplusplus
}
#endif
