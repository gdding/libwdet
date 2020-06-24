#ifdef WIN32
	#define WDET_API extern "C" __declspec(dllexport)
#else
	#define WDET_API
#endif

#include <stdarg.h>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "WordFind.h"
#include "utils/Utility.h"
using namespace ::ns_wdet;

#pragma comment(lib, "utils")


//计算词条的评分
static double computeWordScore(const CWordContext *w)
{
	double dbScore = 0.0;
	dbScore += log(w->nFreq+1.0) * log((double)min(w->nLeftCharCount+1.0, w->nRightCharCount+1.0));
	return dbScore;

	//计算左邻接熵
	double dbLeftEntropy = 0.0;
	unsigned int nTotalLeftFreq = 0;
	for(int i=0; i < w->nLeftCharCount; i++)
	{
		nTotalLeftFreq += w->pnLeftFreqs[i];
	}
	for(int i=0; i < w->nLeftCharCount; i++)
	{
		double p = (double)w->pnLeftFreqs[i]/nTotalLeftFreq;
		dbLeftEntropy += -p*log(p);
	}

	//计算右邻接熵
	double dbRightEntropy = 0.0;
	unsigned int nTotalRightFreq = 0;
	for(int i = 0; i < w->nRightCharCount; i++)
	{
		nTotalRightFreq += w->pnRightFreqs[i];
	}
	for(int i = 0; i < w->nRightCharCount; i++)
	{
		double p = (double)w->pnRightFreqs[i]/nTotalRightFreq;
		dbRightEntropy += -p*log(p);
	}
	
	dbScore += min(dbLeftEntropy, dbRightEntropy);
	dbScore = dbScore * w->dbFirstProb * w->dbFinalProb;
	return dbScore;
}

static bool wordGreaterByScore(const CWordContext *w1, const CWordContext *w2)
{
	return w1->dbScore > w2->dbScore;
}

WDET* wdet_init()
{
	return new CWordFind();
}

WDETcode wdet_setopt(WDET* wdet, WDETOption opt, ...)
{
	CWordFind* wf = (CWordFind*)wdet;
	if(wf == NULL)
		return WDET_BAD_FUNC_PARAM;
	
	WDETcode ret = WDET_OK;
	va_list arg;
	va_start(arg, opt);
	ret = wf->SetOption(opt, arg);
	va_end(arg);
	
	return ret;
}

wdet_word* wdet_perform(WDET* wdet, const char* text, int text_size, int* count)
{
	if(count != NULL) *count = 0;
	if(wdet == NULL || text == NULL || text_size < 0) return NULL;

	//获得结果
	WORD_ARRAY wordArray;
	WDETcode ret = ((CWordFind*)wdet)->extractWord(wordArray,text,text_size);
	if(ret != WDET_OK)
	{
		((CWordFind*)wdet)->FreeWordArray(wordArray);
		return NULL;
	}

	//根据评分对每个串从大到小排序
	WORD_ARRAY::const_iterator it = wordArray.begin();
	for(; it!=wordArray.end(); it++)
	{
		(*it)->dbScore = computeWordScore(*it);
	}
	sort(wordArray.begin(), wordArray.end(), wordGreaterByScore);

	//生成结果
	wdet_word* result = NULL;
	if(!wordArray.empty())
	{
		*count = (int)wordArray.size();
		result = (wdet_word*)calloc(*count, sizeof(wdet_word));
		assert(result != NULL);
		for(int i = 0; i < *count; i++)
		{
			const CWordContext* w = wordArray[i];
			result[i].freq = w->nFreq;
			result[i].leftcount = w->nLeftCharCount;
			result[i].rightcount = w->nRightCharCount;
			result[i].score = w->dbScore;
			strncpy(result[i].word, w->sWord.c_str(), WDET_MAX_WORD_SIZE-1);
		}
	}

	//释放原有结果
	((CWordFind*)wdet)->FreeWordArray(wordArray);

	return result;
}

wdet_word* wdet_perform_file(WDET* h, const char* txtfile, int* count)
{
	wdet_word* result = NULL;
	char* pTxtBuf = NULL;
	int nSize = loadFile(txtfile, pTxtBuf);
	if(nSize >= 0)
	{
		result = wdet_perform(h, pTxtBuf, nSize, count);
		free(pTxtBuf);
	}
	return result;
}

void wdet_free_result(wdet_word* result)
{
	if(result != NULL)
	{
		free(result);
	}
}

void wdet_cleanup(WDET* wdet)
{
	if(wdet != NULL)
		delete ((CWordFind*)wdet);
}

WDETM* wdet_stream_start(WDET* wdet)
{
	if(wdet == NULL) 
		return NULL;
	else
		return ((CWordFind*)wdet)->startIteration();
}

WDETcode wdet_stream_perform(WDET* wdet, WDETM* stream, const char* text, int text_size)
{
	if(wdet == NULL) 
		return WDET_BAD_FUNC_PARAM;
	else
		return ((CWordFind*)wdet)->nextText(stream, text, text_size);
}

WDETcode wdet_stream_perform_file(WDET* wdet, WDETM* stream, const char* txtfile)
{
	char* pTxtBuf = NULL;
	int nSize = loadFile(txtfile, pTxtBuf);
	if(nSize < 0) return WDET_FILE_READ_FAILED;
	WDETcode rt = wdet_stream_perform(wdet, stream, pTxtBuf, nSize);
	free(pTxtBuf);
	return rt;
}

wdet_word* wdet_stream_end(WDET* wdet, WDETM* stream, int* count)
{
	if(count != NULL) *count = 0;
	if(wdet == NULL) return NULL;

	//获得结果
	WORD_ARRAY wordArray;
	((CWordFind*)wdet)->endIteration(stream, wordArray);

	//根据评分对每个串从大到小排序
	WORD_ARRAY::const_iterator it = wordArray.begin();
	for(; it!=wordArray.end(); it++)
	{
		(*it)->dbScore = computeWordScore(*it);
	}
	sort(wordArray.begin(), wordArray.end(), wordGreaterByScore);

	//生成结果
	wdet_word* result = NULL;
	if(!wordArray.empty())
	{
		*count = (int)wordArray.size();
		result = (wdet_word*)calloc(*count, sizeof(wdet_word));
		assert(result != NULL);
		for(int i = 0; i < *count; i++)
		{
			const CWordContext* w = wordArray[i];
			result[i].freq = w->nFreq;
			result[i].leftcount = w->nLeftCharCount;
			result[i].rightcount = w->nRightCharCount;
			result[i].score = w->dbScore;
			strncpy(result[i].word, w->sWord.c_str(), WDET_MAX_WORD_SIZE-1);
		}
	}

	//释放原有结果
	((CWordFind*)wdet)->FreeWordArray(wordArray);

	return result;
}

