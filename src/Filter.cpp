#include "Filter.h"
#include "utils/Utility.h"

namespace ns_wdet
{

#define DEFAULT_STOPCHARFILTER_GBK		"./filter_gbk/stopchar.txt"
#define DEFAULT_STOPCHARFILTER_UTF8		"./filter_utf8/stopchar.txt"

#define DEFAULT_FIRSTCHARFILTER_GBK		"./filter_gbk/firstchar.txt"
#define DEFAULT_FIRSTCHARFILTER_UTF8	"./filter_utf8/firstchar.txt"

#define DEFAULT_FINALCHARFILTER_GBK		"./filter_gbk/finalchar.txt"
#define DEFAULT_FINALCHARFILTER_UTF8	"./filter_utf8/finalchar.txt"

#define DEFAULT_CNSTOPWORDFILTER_GBK	"./filter_gbk/cnstopword.txt"
#define DEFAULT_CNSTOPWORDFILTER_UTF8	"./filter_utf8/cnstopword.txt"
#define DEFAULT_ENSTOPWORDFILTER_GBK	"./filter_gbk/enstopword.txt"
#define DEFAULT_ENSTOPWORDFILTER_UTF8	"./filter_utf8/enstopword.txt"

#define DEFAULT_CHARPROBFILTER_GBK		"./filter_gbk/charprob.txt"
#define DEFAULT_CHARPROBFILTER_UTF8		"./filter_utf8/charprob.txt"


CFilter::CFilter(WDETcharset cs)
{
	m_nCharset = cs;

	m_pStopCharFilter = NULL;
	m_pFirstCharFilter = NULL;
	m_pFinalCharFilter = NULL;

	m_pCharStatInfo = (tCharStatInfo*)calloc(WDET_MAX_CHARSET_SIZE, sizeof(tCharStatInfo));
	assert(m_pCharStatInfo != NULL);
	for(int i=0; i<WDET_MAX_CHARSET_SIZE; i++)
	{
		m_pCharStatInfo[i].chCode = i;
		m_pCharStatInfo[i].dbFirstProb = 1.0;
		m_pCharStatInfo[i].dbFinalProb = 1.0;
	}

	if(m_nCharset == WDET_CHARSET_GBK)
	{
		setStopCharFilterFile(DEFAULT_STOPCHARFILTER_GBK);
		setFirstCharFilterFile(DEFAULT_FIRSTCHARFILTER_GBK);
		setFinalCharFilterFile(DEFAULT_FINALCHARFILTER_GBK);
		setCNStopWordFilterFile(DEFAULT_CNSTOPWORDFILTER_GBK);
		setENStopWordFilterFile(DEFAULT_ENSTOPWORDFILTER_GBK);
	}
	else
	{
		setStopCharFilterFile(DEFAULT_STOPCHARFILTER_UTF8);
		setFirstCharFilterFile(DEFAULT_FIRSTCHARFILTER_UTF8);
		setFinalCharFilterFile(DEFAULT_FINALCHARFILTER_UTF8);
		setCNStopWordFilterFile(DEFAULT_CNSTOPWORDFILTER_UTF8);
		setENStopWordFilterFile(DEFAULT_ENSTOPWORDFILTER_UTF8);
	}
}

CFilter::~CFilter()
{
	m_filterWordSet.clear();
	m_filterWordSetEn.clear();
	if(m_pStopCharFilter != NULL)
		free(m_pStopCharFilter);
	if(m_pFirstCharFilter != NULL)
		free(m_pFirstCharFilter);
	if(m_pFinalCharFilter != NULL)
		free(m_pFinalCharFilter);
	if(m_pCharStatInfo != NULL)
	{
		free(m_pCharStatInfo);
		m_pCharStatInfo = NULL;
	}
}

WDETcode CFilter::setCharset(WDETcharset cs)
{
	if(m_nCharset != cs)
	{
		m_nCharset = cs;
		if(m_nCharset == WDET_CHARSET_GBK)
		{
			setStopCharFilterFile(DEFAULT_STOPCHARFILTER_GBK);
			setFirstCharFilterFile(DEFAULT_FIRSTCHARFILTER_GBK);
			setFinalCharFilterFile(DEFAULT_FINALCHARFILTER_GBK);
			setCNStopWordFilterFile(DEFAULT_CNSTOPWORDFILTER_GBK);
			setENStopWordFilterFile(DEFAULT_ENSTOPWORDFILTER_GBK);
		}
		else
		{
			setStopCharFilterFile(DEFAULT_STOPCHARFILTER_UTF8);
			setFirstCharFilterFile(DEFAULT_FIRSTCHARFILTER_UTF8);
			setFinalCharFilterFile(DEFAULT_FINALCHARFILTER_UTF8);
			setCNStopWordFilterFile(DEFAULT_CNSTOPWORDFILTER_UTF8);
			setENStopWordFilterFile(DEFAULT_ENSTOPWORDFILTER_UTF8);
		}
	}
	
	return WDET_OK;
}

WDETcode CFilter::setStopCharFilterFile(const char* filename)
{
	if(m_pStopCharFilter != NULL)
	{
		free(m_pStopCharFilter);
		m_pStopCharFilter = NULL;
	}
	m_pStopCharFilter = (char*)calloc(WDET_MAX_CHARSET_SIZE, sizeof(char));
	if(m_pStopCharFilter == NULL) 
		return WDET_OUT_OF_MEMORY;
	else
		return setFlag(filename, m_pStopCharFilter);
}

WDETcode CFilter::setFirstCharFilterFile(const char* filename)
{
	if(m_pFirstCharFilter != NULL)
	{
		free(m_pFirstCharFilter);
		m_pFirstCharFilter = NULL;
	}
	m_pFirstCharFilter = (char*)calloc(WDET_MAX_CHARSET_SIZE, sizeof(char));
	if(m_pFirstCharFilter == NULL) 
		return WDET_OUT_OF_MEMORY;
	return setFlag(filename, m_pFirstCharFilter);
}

WDETcode CFilter::setFinalCharFilterFile(const char* filename)
{
	if(m_pFinalCharFilter != NULL)
	{
		free(m_pFinalCharFilter);
		m_pFinalCharFilter = NULL;
	}
	m_pFinalCharFilter = (char*)calloc(WDET_MAX_CHARSET_SIZE, sizeof(char));
	if(m_pFinalCharFilter == NULL)
		return WDET_OUT_OF_MEMORY;
	return setFlag(filename, m_pFinalCharFilter);
}

WDETcode CFilter::setCharProbFile(const char* filename)
{
	static const double INITIAL_PROB = 1.0;
	char* pcBuffer = NULL;
	int nFileSize = loadEncryptFile(filename, pcBuffer);
	if(nFileSize < 0) return WDET_FILE_READ_FAILED;

	vector<string> lines;
	parseTextByLine(pcBuffer, nFileSize, lines);
	for(size_t i = 0; i < lines.size(); i++)
	{
		const string& sLine = lines[i];
		char sWord[1024] = {0};
		char sFirstProb[100] = {0};
		char sFinalProb[100] = {0};
		sscanf(sLine.c_str(), "%s %s %s", sWord, sFirstProb, sFinalProb);
		if (sWord[0]==0) continue;
		double dbFirstProb = atof(sFirstProb);
		double dbFinalProb = atof(sFinalProb);
		int nPos = 0;
		int nChCode = getNextCharCode(sWord, 100, m_nCharset, nPos);
		m_pCharStatInfo[nChCode].dbFirstProb = dbFirstProb + INITIAL_PROB;
		m_pCharStatInfo[nChCode].dbFinalProb = dbFinalProb + INITIAL_PROB;
	}
	free(pcBuffer);

	return WDET_OK;
}

WDETcode CFilter::setCNStopWordFilterFile(const char* filename)
{
	m_filterWordSet.clear();
	char* pcBuffer = NULL;
	int nFileSize = loadEncryptFile(filename, pcBuffer);
	if(nFileSize < 0) return WDET_FILE_READ_FAILED;
	vector<string> lines;
	parseTextByLine(pcBuffer, nFileSize, lines);
	for(size_t i = 0; i < lines.size(); i++)
	{
		const string& sWord = lines[i];
		m_filterWordSet.insert(sWord);
	}
	free(pcBuffer);

	return WDET_OK;
}

WDETcode CFilter::setENStopWordFilterFile(const char* filename)
{
	m_filterWordSetEn.clear();
	char* pcBuffer = NULL;
	int nFileSize = loadEncryptFile(filename, pcBuffer);
	if(nFileSize < 0) return WDET_FILE_READ_FAILED;
	vector<string> lines;
	parseTextByLine(pcBuffer, nFileSize, lines);
	for(size_t i = 0; i < lines.size(); i++)
	{
		const string& sWord = lines[i];
		m_filterWordSetEn.insert(sWord);
	}
	free(pcBuffer);

	return WDET_OK;
}

WDETcode CFilter::setFlag(const char* sFilterFile, char* pCharFilter)
{
	if(pCharFilter == NULL) 
		return WDET_BAD_FUNC_PARAM;

	bool *pbCharVec = NULL;
	WDETcode rt = readFilterCharFile(sFilterFile, pbCharVec);
	if(rt != WDET_OK)
	{
		if(pbCharVec) free(pbCharVec);
		return rt;
	}
	for(int i=0; i<WDET_MAX_CHARSET_SIZE; i++)
	{
		if(pbCharVec[i])
		{
			pCharFilter[i] = 0x01;
		}
	}
	free(pbCharVec);

	return WDET_OK;
}

WDETcode CFilter::readFilterCharFile(const char* sFilterCharFile, bool* & pbCharVec)
{
	if(sFilterCharFile == NULL)
	{
		return WDET_BAD_FUNC_PARAM;
	}
	pbCharVec = (bool*)calloc(WDET_MAX_CHARSET_SIZE, sizeof(bool));
	if(pbCharVec == NULL) return WDET_OUT_OF_MEMORY;
	for(int i=0; i<WDET_MAX_CHARSET_SIZE; i++)
	{
		pbCharVec[i] = false;
	}

	//从文件中读入数据到缓冲区 -> pBuffer
    char *pBuffer = NULL;
    int nFileSize = loadEncryptFile(sFilterCharFile, pBuffer);
    if(nFileSize < 0)
	{
		return WDET_FILE_READ_FAILED;
	}

	//将每个字/字符的内码存入
	int nPos = 0;
	while(nPos < nFileSize)
	{
		int nInnerCode = getNextCharCode(pBuffer, nFileSize, m_nCharset, nPos);
		if(-1==nInnerCode) continue;
		pbCharVec[nInnerCode] = true;
	}
	free(pBuffer);

	return WDET_OK;
}

}

