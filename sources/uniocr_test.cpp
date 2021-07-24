#include <string>
#include <iostream>
#include "rpuniocr_api.h"
#define TESTIMG  "images\\1000.jpg"
#define MODELDIR "models"


int main(int argc,  char * argv[])
{

	 const char * szImg;
	 const char* szModelDir;

	 if (argc == 1)
	 {
		 std::cout << "Usage: " << std::endl;
		 std::cout << argv[0] << "  /path/to/model/dir /path/to/img/file   uclip(float-1.2)  box_thresh(float-0.3)  minsidelen(int-200) maxsidelen(int-4000)  text_score(float-0.5)" << std::endl;
		 return -1;
	 }
	 if (argc > 2)
	 {
		 szModelDir = argv[1];
		 szImg = argv[2];
	 }
	 else
	 {
		 szModelDir = MODELDIR;
		 szImg = TESTIMG;
	 }

	 float uclip = 1.2f;

	 if (argc > 3)
	 {
		 uclip = atof(argv[3]);
	 }

	 float boxthresh = 0.3f;

	 if (argc > 4)
	 {
		 boxthresh = atof(argv[4]);
	 }
	

	 int minsidelen = 200;
	 if (argc > 5)
	 {
		 minsidelen = atoi(argv[5]);
	 }

	 int maxsidelen = 4000;
	 if (argc > 6)
	 {
		 maxsidelen = atoi(argv[6]);
	 }


	 float fThresh = 0.5f;
	 if (argc > 7)
	 {
		 fThresh = atof(argv[7])>0? atof(argv[7]) : 0.5f;
	 }



	RP_OCR_INIT_INFO InitInfo={0};

	RPOcrPrepare(&InitInfo);

	InitInfo.OcrParam.max_side_len = 4000;
	InitInfo.OcrParam.det_db_unclip_ratio = 1.2f;
	InitInfo.OcrParam.det_db_box_thresh = 0.3f;

	if (uclip > 0.f)
		InitInfo.OcrParam.det_db_unclip_ratio = uclip;

	if (boxthresh > 0.f)
	{
		InitInfo.OcrParam.det_db_box_thresh = boxthresh;
	}
	if (minsidelen > 0)
	{
		InitInfo.OcrParam.min_side_len = minsidelen;
	}

	if (maxsidelen > 0)
	{
		InitInfo.OcrParam.max_side_len = maxsidelen;
	}

	InitInfo.Lang = QOL_ALL;
	InitInfo.nThreads = 0;
	InitInfo.OcrMode = QOMT_RAPID;
	InitInfo.PipLine = QOP_OCR_ALL;
	InitInfo.szDetModelDir = szModelDir;
	InitInfo.bForceCPU = FALSE;
	
	InitInfo.nGUPID = 0;
	InitInfo.bDebugInfo = TRUE;

	auto OcrHandle = RPOcrInit(&InitInfo);
	if (!OcrHandle)
		return -1;

	bool bSuccess = false;

	int nTimes = 100;

#ifdef NDEBUG

	nTimes = 1;
#endif

	//if (argc > 3)
	//	nTimes = atoi(argv[3]);
	nTimes = 1;
	for (int i = 0; i < nTimes; i++)
	{
		auto pResult = RPOcrDoOcr(OcrHandle, szImg);

		if (!pResult)
			return -1;
		

		//std::string strOut;
		if (pResult)
		{
			int nLen = RPOcrGetLen(pResult);

			if (nLen > 0)
			{

				char* pString = new char[nLen];
				if (RPOcrGetResult(pResult, pString, nLen, fThresh) > 0)
				{
					auto Lang = RPOcrGetLang(pResult);
					std::cout << "LangType: " << Lang << std::endl;
					std::cout << pString << std::endl;
				}



				delete[]pString;
			}

			//// get the location of every box
			//int nBox = RPOCrGetBlockNum(pResult);
			//for (int s = 0; s < nBox; s++)
			//{
			//	auto block = RPOcrGetBlock(pResult, s);
			//	std::cout << block.szString << std::endl;

			//}

			RPOcrFree(pResult);
			
			bSuccess = true;
		}

		//std::cout << "no. " << i << std::endl;

	}



	RPOcrDeinit(OcrHandle);

  	return  bSuccess ?0:-1;
}

