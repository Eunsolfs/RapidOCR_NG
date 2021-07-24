#pragma once 
extern "C"
{
#include "sheenbidi/SheenBidi.h"
}
class CRpBiDiUnicode {

public:
	void static utf8rev(char* str)
	{
		/* this assumes that str is valid UTF-8 */
		char* scanl, * scanr, * scanr2, c;

		/* first reverse the string */
		for (scanl = str, scanr = str + strlen(str); scanl < scanr;)
			c = *scanl, * scanl++ = *--scanr, * scanr = c;

		/* then scan all bytes and reverse each multibyte character */
		for (scanl = scanr = str; c = *scanr++;) {
			if ((c & 0x80) == 0) // ASCII char
				scanl = scanr;
			else if ((c & 0xc0) == 0xc0) { // start of multibyte
				scanr2 = scanr;
				switch (scanr - scanl) {
				case 4: c = *scanl, *scanl++ = *--scanr, *scanr = c; // fallthrough
				case 3: // fallthrough
				case 2: c = *scanl, *scanl++ = *--scanr, *scanr = c;
				}
				scanr = scanl = scanr2;
			}
		}
	}
#ifndef WIN32
#define _strdup strdup
#endif
	static string RevertUtf8(string& strInput)
	{
		char * szNewStr=_strdup(strInput.c_str());
		utf8rev(szNewStr);
		string strNewString = szNewStr;
		free(szNewStr);
		return strNewString;
	}
	static string  ConvertBiDiSheen(const char* szOriText)  //utf-8
	{

		string strOriText = szOriText;
		if (strOriText.length() == 0)
			return strOriText;
		string strOutText = "";
		SBCodepointSequence codepointSequence = { SBStringEncodingUTF8, (void*)strOriText.c_str(), strOriText.length() };

		/* Extract the first bidirectional paragraph. */
		SBAlgorithmRef bidiAlgorithm = SBAlgorithmCreate(&codepointSequence);
		SBParagraphRef firstParagraph = SBAlgorithmCreateParagraph(bidiAlgorithm, 0, INT32_MAX, SBLevelDefaultLTR);
		SBUInteger paragraphLength = SBParagraphGetLength(firstParagraph);

		/* Create a line consisting of whole paragraph and get its runs. */
		SBLineRef paragraphLine = SBParagraphCreateLine(firstParagraph, 0, paragraphLength);
		SBUInteger runCount = SBLineGetRunCount(paragraphLine);
		const SBRun* runArray = SBLineGetRunsPtr(paragraphLine);

		/* Log the details of each run in the line. */
		string strTmp;
		for (SBUInteger i = 0; i < runCount; i++) {


			switch (runArray[i].level)
			{
			case SBBidiTypeL:
				 strTmp = strOriText.substr(runArray[i].offset, runArray[i].length);
				 // need to revert
				 strOutText += RevertUtf8(strTmp);
				break;
			case SBBidiTypeR:
				 strTmp = strOriText.substr(runArray[i].offset, runArray[i].length);
				 strOutText += strTmp;
				break;
/*			case SBLevelDefaultRTL:
				strTmp = strOriText.substr(runArray[i].offset, runArray[i].length);
				break;
			case SBLevelDefaultLTR:
				strTmp = strOriText.substr(runArray[i].offset, runArray[i].length);
				break*/;
			default:
				 strTmp = strOriText.substr(runArray[i].offset, runArray[i].length);
				 strOutText += strTmp;

				
			}


			//cout << strTmp << std::endl;

			//printf("Run Offset: %ld\n", (long)runArray[i].offset);
			//printf("Run Length: %ld\n", (long)runArray[i].length);
			//printf("Run Level: %ld\n\n", (long)runArray[i].level);
		}

		//SBMirrorLocatorRelease(mirrorLocator);
		SBLineRelease(paragraphLine);
		SBParagraphRelease(firstParagraph);
		SBAlgorithmRelease(bidiAlgorithm);

		return 		strOutText;
	}



};