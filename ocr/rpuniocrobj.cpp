#include "precomp.h"

CRpUniOCRObj::CRpUniOCRObj(RP_OCR_INIT_INFO* InitInfo,bool bUseGPU)
{
	
	m_bUseGPU = bUseGPU;
	exp_bad_param except;
	if (!InitInfo->szDetModelDir || InitInfo->szDetModelDir[0]== 0)
	{
		throw except;
	}
	m_strModeDir = InitInfo->szDetModelDir;
	if (m_strModeDir.substr(m_strModeDir.length() - 1) != OS_SEP)
		m_strModeDir += OS_SEP;
	m_InitInfo = *InitInfo;

}

CRpUniOCRObj::~CRpUniOCRObj()
{

	
	for (auto& item : m_RecObjLst)
	{
		if (item.first != QOLO_OTHER  && item.second)
		{
			delete item.second;
			item.second = nullptr;
		}
	}

}

void CRpUniOCRObj::LoadDefaultParam(RPOCR_PARAM* Param)
{

	/*
	          const int num_thread=4,
                      const int max_side_len= 960,
                      const double det_db_thresh= 0.3,
                      const double det_db_box_thresh= 0.5,
                      const double det_db_unclip_ratio = 1.9,   // box的边大小,default 2.0
                      const bool use_polygon_score=true
	*/

	Param->cls_thresh = 0.5f;
	Param->det_db_box_thresh = 0.5f;
	Param->max_side_len = 4000;
	Param->min_side_len = 200;
	Param->det_db_thresh = 0.3f;
	Param->det_db_unclip_ratio = 1.2f; // 1.8f
	Param->use_polygon_score = 1;
	Param->strength_degree = 0;  

}

void CRpUniOCRObj::SetDefaultParam()
{

	LoadDefaultParam(&m_Param);
		
}

void CRpUniOCRObj::SetParam(RPOCR_PARAM* Param)
{

	m_Param = *Param;
}

bool CRpUniOCRObj::InitOcrEngine()
{

	//PipeLine


	if (m_InitInfo.PipLine & QOP_OCR) // default 
	{
		// init det

		string strDetModel{ m_strModeDir };

		strDetModel += G_OCR_DET_MODE[m_InitInfo.OcrMode];

		m_DetObj.Init(strDetModel,m_InitInfo.nThreads,m_InitInfo.OcrParam.max_side_len, m_InitInfo.OcrParam.min_side_len,m_InitInfo.OcrParam.det_db_thresh,m_InitInfo.OcrParam.det_db_box_thresh,m_InitInfo.OcrParam.det_db_unclip_ratio,m_InitInfo.OcrParam.use_polygon_score, m_bUseGPU, m_InitInfo.nGUPID);

		string strClsModel = m_strModeDir + RP_OCR_CLS_MODEL;
		m_ClsObj.Init(strClsModel, m_InitInfo.nThreads, m_InitInfo.OcrParam.cls_thresh, m_bUseGPU, m_InitInfo.nGUPID);
	
	   // Initialize ocr engine per languages.
		//QOL_CH_ENG = 1,
		//	QOL_KOREAN = 2,
		//	QOL_MONGOLIA = 4,
		//	QOL_UYGUR = 8,
		//	QOL_TIBET = 16,
		if (m_InitInfo.Lang == 0)
		{
			printf("Not set language type to be loaded, by default it is all. ");
			m_InitInfo.Lang = QOL_ALL;
		}


		for (size_t i = 0; i < MAX_LANG_SCOPE; i++)
		{

			m_RecObjLst[(RP_OCR_LANG_OFFSET)i] = nullptr;
			if (m_InitInfo.Lang & (0x1 << i))
			{
				m_RecObjLst[(RP_OCR_LANG_OFFSET)i] = new RapidOCR::CRpOCRRec;
				string strModelPath, strModelDict;
				strModelPath = m_strModeDir + RP_OCR_REC_MODEL[i];
				strModelDict = m_strModeDir + RP_OCR_REC_DICT[i];
				m_RecObjLst[(RP_OCR_LANG_OFFSET)i]->Init(strModelPath, m_InitInfo.nThreads, strModelDict,RP_OCR_LANG_OFFSET(i),m_bUseGPU, m_InitInfo.nGUPID,m_InitInfo.nBatchSize);

			}
		}

		m_RecObjLst[MAX_LANG_SCOPE] = m_RecObjLst[m_InitInfo.DefaultLang];  //default ocr engine.


	}

	return true;
}


RapidOCR::RAPID_OCR_RESULT* CRpUniOCRObj::DoOCR(cv::Mat SrcImg)
{

	RapidOCR::TXTBOX Boxes;
	m_DetObj.Run(SrcImg, Boxes);
	if (Boxes.size() == 0)
		return nullptr; // not found text

	int nNeedBoxes = Boxes.size();
	if (Boxes.size() > MAX_LANG_DET_BOX_NUMBER)
		nNeedBoxes = MAX_LANG_DET_BOX_NUMBER;
	RapidOCR::TXTBOX	DetBoxes(Boxes.begin(), Boxes.begin() + nNeedBoxes);
	// debug only

	if(m_InitInfo.bDebugInfo)
		RapidOCR::Utility::VisualizeBboxes(SrcImg, DetBoxes);

	
	RapidOCR::RAPID_OCR_RESULT* pResult = new RapidOCR::RAPID_OCR_RESULT;
	auto Type = QOLO_CH_ENG; // by default, it's chinese-english
	RapidOCR::CRpOCRRec* pRecObj = m_RecObjLst[Type];

	// for bidisplay: https://github.com/Tehreer/SheenBidi
	if (pRecObj)
	{
		pResult->Lang = Type;
		pRecObj->Run(Boxes, SrcImg, &m_ClsObj,*pResult);
		return pResult;
	}
	else
	{
		delete pResult;
		return nullptr;
	}


		
	
}


void CRpUniOCRObj::FreeResult(RapidOCR::RAPID_OCR_RESULT* pResult)
{
	if (pResult)
		delete pResult;

}

string CRpUniOCRObj::GetLangName(RP_OCR_LANG_TYPE Lang)
{

	return RP_LANG_NAME[Lang];
}



bool Less(const LINESECTION& s1, const LINESECTION& s2)
{
	return s1->nRCenter < s2->nRCenter; //从小到大排序
}

bool CRpUniOCRObj::ReCompose(RapidOCR::RAPID_OCR_RESULT& Result, vector<LINE> & AllLines)
{
	

	bool bVertical = Result.Lang == QOLO_MONGOLIA;

	// if bVertical == true , get the middle of X, otherwise get the middle of Y

	/*
	[x][0]->x,  [x][1]->y

	0       1
	3		2


	per box, [0]->x, [1]->y
	*/
	const int threshhold = 10; // 中心点最多误差10个坐标点
	vector<int> LineMark;
	bool bHave = false;
	int nCenter = 0, nHCenter, nVCenter;

	//根据中心点找出行数或列数 based on language type
	for (auto& Item : Result.Result)
	{
		nHCenter = Item.Box[0][0] + (Item.Box[2][0] - Item.Box[0][0]) / 2; //x
		nVCenter = Item.Box[0][1] + (Item.Box[2][1] - Item.Box[0][1]) / 2; // y

		if (bVertical)
		{
			nCenter = nHCenter;
			Item.nRCenter = nVCenter;
		}
		else
		{
			nCenter = nVCenter ;
			Item.nRCenter = nHCenter;
		}


		bHave = false;
		for (auto mark : LineMark)
		{
			if (nCenter > (mark - threshhold) && nCenter < (mark + threshhold))
			{
				bHave = true;
				break;
			}
		}
				
		if (!bHave)
			LineMark.push_back(nCenter);

		//cout << Item.Box.size() << endl;   // 0 x, 1 y//  0 left-top, 1 right-top,2 right-bottom, 3 left-bottom
	}
	// 重排版函数，使用box的中线进行行或列对准，然后将文字归为同一行或同一列

	sort(LineMark.begin(), LineMark.end()); //从小到大进行行或列排序 

	for (auto& Item : Result.Result)
	{

		for (int i = 0; i < LineMark.size(); i++)
		{
			if (AllLines.size() <= i)
			{
				LINE tmp;
				AllLines.push_back(tmp);
			}

			if (bVertical)
				nCenter = Item.Box[0][0] + (Item.Box[2][0] - Item.Box[0][0]) / 2;
			else
				nCenter = Item.Box[0][1] + (Item.Box[2][1] - Item.Box[0][1]) / 2;

			if (nCenter > (LineMark[i] - threshhold) && nCenter < (LineMark[i] + threshhold))
			{
		
				AllLines[i].push_back(&Item);
				break;
			}
		}
	}

	// resort text per line
	for (auto& Item : AllLines)
	{
		sort(Item.begin(), Item.end(), Less);
	}



	return true;
}