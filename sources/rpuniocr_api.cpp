#include "precomp.h"
#ifdef __cplusplus
extern "C"
{
#endif 


	_RP_OCR_API void   RPOcrPrepare(RP_OCR_INIT_INFO* InitInfo)
	{

		CRpUniOCRObj::LoadDefaultParam(&InitInfo->OcrParam);
	}
	_RP_OCR_API RPHANDLE   RPOcrInit(RP_OCR_INIT_INFO* InitInfo)
	{

		bool bUseGPU = false;
		InitInfo->nGUPID = 0;
#ifdef WIN32
		RP_GPUINFO GPUInfo={ -1,0 };
#endif
#if defined(USE_DML) 
		if (!InitInfo->bForceGPU)
		{
			if (IsDMLAvailable())
			{
				bUseGPU = true;

			}

			if (bUseGPU)
			{
				 GPUInfo= QmSelectGPU();
				 InitInfo->nGUPID = GPUInfo.nGPUID;
				if (InitInfo->nGUPID < 0)
					bUseGPU = false; // not found appropriate GPU ( other than INTEL GPU)


			}
		}
		else
		{

			bUseGPU = true;
			//InitInfo->nGUPID = 0; // being forced to 0
		}

#endif

#if defined(USE_CUDA)

		bUseGPU = true;
#endif
		if (InitInfo->bForceCPU)
			bUseGPU = false;






		if (bUseGPU)
		{
		
#ifdef WIN32
			InitInfo->nBatchSize = ceil((GPUInfo.nMemCap+1 )/ 1024 / 1.5) * 6;
#else
			InitInfo->nBatchSize = 6;
#endif
		}
		else
			InitInfo->nBatchSize = 6;


		auto pNewObj=new 	CRpUniOCRObj(InitInfo, bUseGPU);

		bool bResult = pNewObj->InitOcrEngine();
		if (bResult)
			return pNewObj;
		else
		{
			delete pNewObj;
			return nullptr;
		}
	}

	_RP_OCR_API   RPRETHANDLE RPOcrDoOcr(RPHANDLE handle, const char* szImgPath)
	{
		std::ifstream  ff(szImgPath, std::ios_base::binary);
		if (!ff.is_open())
			return nullptr;
		std::stringstream  sImg;
		sImg << ff.rdbuf();

		return RPOcrDoOcrMem(handle, (const char *)sImg.str().data(), sImg.str().size());

	}


	_RP_OCR_API   RPRETHANDLE RPOcrDoOcrMem(RPHANDLE handle, const char* pImgBuf, int nLen)
	{

		cv::Mat srcImg; // = CRpImgRead::QmReadImg((const BYTE*)pImgBuf, nLen);
		int nChannel = srcImg.channels();

		CRpUniOCRObj* pObj = (CRpUniOCRObj*)handle;
		if (pObj)
		{
			auto pResult = pObj->DoOCR(srcImg);
			if(pResult)
				pResult->pObj = handle;
			return pResult;
		}

		return nullptr;

	}

	_RP_OCR_API  int RPOcrGetLen(RPRETHANDLE handle)
	{
		RapidOCR::RAPID_OCR_RESULT* pResult =(RapidOCR::RAPID_OCR_RESULT * )handle;
		if (!pResult)
			return 0;
		int nStrLen=0;
		for (auto& Item : pResult->Result)
		{
	/*		if (Item.score < nScore)
				continue;*/

			nStrLen += Item.strTxt.length();
			nStrLen ++;
		}

		return ++nStrLen;
	}

	_RP_OCR_API  int RPOcrGetResult(RPRETHANDLE handle, char* szBuf, int nLen, float nScore)
	{
		RapidOCR::RAPID_OCR_RESULT* pResult = (RapidOCR::RAPID_OCR_RESULT*)handle;
		if (!pResult)
			return 0;
		
		string strAll = "";

		// re-compose the output of texts.

		CRpUniOCRObj* pObj = (CRpUniOCRObj*)pResult->pObj;
		if (!pObj)
			return 0;

		vector<LINE> AllLines;
		bool bRet= pObj->ReCompose(*pResult, AllLines);

		string strLine;
		for (auto& Item : AllLines)
		{
			strLine = "";
			for (auto& Section : Item)
			{
				if (Section->score < nScore)
					continue;
				strLine += Section->strTxt;
			}
			if (pResult->Lang == QOLO_UYGUR)
				strLine = CRpBiDiUnicode::ConvertBiDiSheen(strLine.c_str());

			strAll += strLine;
			strAll += "\n";
	
		}
		if (nLen < strAll.length())
			return 0;
		strncpy(szBuf, strAll.c_str(), strAll.length());
		szBuf[strAll.length()] = '\0';
		return  strAll.length();
	}

	_RP_OCR_API  RP_OCR_LANG_TYPE RPOcrGetLang(RPRETHANDLE handle)
	{
		RapidOCR::RAPID_OCR_RESULT* pResult = (RapidOCR::RAPID_OCR_RESULT*)handle;
		if (!pResult)
			return QOLO_NONE;

		return pResult->Lang;

	}

	_RP_OCR_API  void RPOcrFree(RPRETHANDLE handle)
	{

		RapidOCR::RAPID_OCR_RESULT* pResult = (RapidOCR::RAPID_OCR_RESULT*)handle;
		if (pResult)
			delete pResult;

	}
	_RP_OCR_API void  RPOcrDeinit(RPHANDLE handle)
	{
		auto p = (CRpUniOCRObj*)handle;
		if (p)
			delete p;

	}


	// for the affiliate of  data security  only

	_RP_OCR_API  int RPOCrGetBlockNum(RPRETHANDLE handle)
	{
		RapidOCR::RAPID_OCR_RESULT* pResult = (RapidOCR::RAPID_OCR_RESULT*)handle;
		if (!pResult)
			return 0;

		return pResult->Result.size();

	}

	_RP_OCR_API  RPOCR_DET_BLOCK RPOcrGetBlock(RPRETHANDLE handle, int nIndex)
	{
		RPOCR_DET_BLOCK Ret{};
		RapidOCR::RAPID_OCR_RESULT* pResult = (RapidOCR::RAPID_OCR_RESULT*)handle;
		if (!pResult)
			return Ret;

		Ret.szString = pResult->Result[nIndex].strTxt.c_str();
		//  0 left-top, 1 right-top,2 right-bottom, 3 left-bottom  
		
		/*
		0    1
		3    2

		per location: [0]->x, [1]->y
		*/
		
		Ret.Left = pResult->Result[nIndex].Box[0][0];
		Ret.Top = pResult->Result[nIndex].Box[0][1];
		Ret.Right = pResult->Result[nIndex].Box[2][0];
		Ret.Bottom = pResult->Result[nIndex].Box[2][1];
		
		return Ret;
	}

#ifdef __cplusplus
}
#endif 