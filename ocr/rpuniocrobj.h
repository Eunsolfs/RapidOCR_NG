#pragma once

typedef map< RP_OCR_LANG_OFFSET, RapidOCR::CRpOCRRec *> OCRRECOBJ;

typedef  RapidOCR::RP_OCR_RESULT_ITEM* LINESECTION;
typedef  vector< LINESECTION> LINE;


class CRpUniOCRObj
{
	
private:

	RapidOCR::CRpOCRCls			m_ClsObj;  // direction detection
	RapidOCR::CRpOCRDet			m_DetObj;  // character detection


	RP_OCR_LANG_TYPE			m_curLangType;

	RPOCR_PARAM					m_Param;
	RP_OCR_INIT_INFO			m_InitInfo;

	string						m_strModeDir;

	OCRRECOBJ					m_RecObjLst; // character recognition

	bool						m_bUseGPU = false;
public:

	CRpUniOCRObj(RP_OCR_INIT_INFO * InitInfo, bool bUseGPU=false);
	~CRpUniOCRObj();

	static void LoadDefaultParam(RPOCR_PARAM* Param);
	 void SetParam(RPOCR_PARAM* Param);
	void SetDefaultParam();
	bool InitOcrEngine();
	RapidOCR::RAPID_OCR_RESULT * DoOCR(cv::Mat SrcImg);
	
	void FreeResult(RapidOCR::RAPID_OCR_RESULT * pResult);
	string GetLangName(RP_OCR_LANG_TYPE Lang);

	bool ReCompose(RapidOCR::RAPID_OCR_RESULT& Result, vector<LINE> & AllLines);


};