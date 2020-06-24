#include "WordFind.h"
#include "Filter.h"
#include "utils/Utility.h"

namespace ns_wdet
{

#define MAX_TEXT_LENGTH		(2*1024*1024)
#define ALPHABET_OR_NUMBER(c) ((c<='z'&&c>='a')||(c<='Z'&&c>='A')||(c<='9'&&c>='0'))

bool wordGreaterByString(const CWordContext *w1, const CWordContext *w2)
{
	return w1->sWord > w2->sWord;
}


CWordFind::CWordFind()
{
	m_nCharset = WDET_CHARSET_GBK;
	m_nMaxTextLen = MAX_TEXT_LENGTH;
	m_nMaxWordLen = 10;
	m_nAVThresh = 3;
	m_pFilter = new CFilter(m_nCharset);
	m_pfOldWord = NULL;

	//初始化树组m_pCharTreeA0
	m_pCharTreeA0 = (tCharTreeNode*)calloc(WDET_MAX_CHARSET_SIZE, sizeof(tCharTreeNode));
	assert(m_pCharTreeA0 != NULL);
	memset(m_pCharTreeA0, 0, sizeof(tCharTreeNode)*WDET_MAX_CHARSET_SIZE);
	for(unsigned int i = 0; i < WDET_MAX_CHARSET_SIZE; i++)
	{
		m_pCharTreeA0[i].chCode = i;
	}
}

CWordFind::~CWordFind()
{
	if(m_pFilter != NULL)
		delete m_pFilter;
	if(m_pCharTreeA0 != NULL)
		free(m_pCharTreeA0);
}

WDETcode CWordFind::SetOption(WDETOption opt, va_list param)
{
	WDETcode ret = WDET_OK;
	int nParam = 0;
	const char* sParam = NULL;
	switch(opt) 
	{
	case WDETOPT_CHARSET:
		nParam = va_arg(param, int);
		if(nParam != WDET_CHARSET_GBK && nParam != WDET_CHARSET_UTF8)
		{
			ret = WDET_INVALID_OPTION;
			break;
		}
		else
		{
			m_nCharset = (WDETcharset)nParam;
			m_pFilter->setCharset(m_nCharset);
			break;
		}
	case WDETOPT_MAX_WORD_LENGTH:
		nParam = va_arg(param, int);
		if(nParam > 0 && nParam < 64)
			m_nMaxWordLen = nParam;
		break;
	case WDETOPT_MIN_AVTHRESH:
		nParam = va_arg(param, int);
		if(nParam > 0)
			m_nAVThresh = nParam;
		break;
	case WDETOPT_WORD_EXISTED_FUNC:
		m_pfOldWord = va_arg(param, cb_existed_word);
		break;
	case WDETOPT_STOPCHAR_FILTER_FILE:
		sParam = va_arg(param, char*);
		ret = m_pFilter->setStopCharFilterFile(sParam);
		break;
	case WDETOPT_STOPWORD_FILTER_FILE:
		sParam = va_arg(param, char*);
		ret = m_pFilter->setCNStopWordFilterFile(sParam);
		break;
	case WDETOPT_FIRSTCHAR_FILTER_FILE:
		sParam = va_arg(param, char*);
		ret = m_pFilter->setFirstCharFilterFile(sParam);
		break;
	case WDETOPT_FINALCHAR_FILTER_FILE:
		sParam = va_arg(param, char*);
		ret = m_pFilter->setFinalCharFilterFile(sParam);
		break;
	case WDETOPT_CHARPROB_FILTER_FILE:
		sParam = va_arg(param, char*);
		ret = m_pFilter->setCharProbFile(sParam);
		break;
	default:
		ret = WDET_INVALID_OPTION;
	}
	
	return ret;
}

void CWordFind::FreeWordArray(WORD_ARRAY &wordArray)
{
	WORD_ARRAY::iterator it = wordArray.begin();
	for(; it != wordArray.end(); it++)
	{
		if((*it)->pLeftChars != NULL)
			free((*it)->pLeftChars); 
		if((*it)->pRightChars != NULL)
			free((*it)->pRightChars);
		if((*it)->pnLeftFreqs != NULL)
			free((*it)->pnLeftFreqs);
		if((*it)->pnRightFreqs != NULL)
			free((*it)->pnRightFreqs);
		delete (*it);
	}
	wordArray.clear();
}

TextStream* CWordFind::startIteration()
{
	struct stTextStream* pstTextStream = new struct stTextStream;
	if(pstTextStream != NULL)
	{
		pstTextStream->vecWordContext.clear();
		pstTextStream->pText = (char*)calloc(m_nMaxTextLen, sizeof(char));
		assert(pstTextStream->pText != NULL);
		pstTextStream->nTextLen = 0;
	}
	return pstTextStream;
}

WDETcode CWordFind::nextText(TextStream* h, const char* sText, int nLen)
{
	//检查参数合法性
	if(h == NULL || sText == NULL || nLen < 0) return WDET_BAD_FUNC_PARAM;
	struct stTextStream *pStream = (struct stTextStream*)h;

	//文本长度超长的直接处理
	if(nLen >= m_nMaxTextLen)
	{
		//抽取词条 -> curWordArray
		WORD_ARRAY curWordArray;
		extractWord(curWordArray, sText, nLen);

		//与原有抽取结果合并
		mergeWordArray(pStream->vecWordContext, curWordArray);
		return WDET_OK;
	}

	//若缓冲区剩余空间不足以缓存该文本则先对缓冲区中的文本进行处理
	if(pStream->nTextLen + nLen >= m_nMaxTextLen)
	{
		//抽取词条 -> curWordArray
		WORD_ARRAY curWordArray;
		extractWord(curWordArray, pStream->pText, pStream->nTextLen);

		//与原有抽取结果合并
		mergeWordArray(pStream->vecWordContext, curWordArray);

		memset(pStream->pText, 0, m_nMaxTextLen*sizeof(char));
		pStream->nTextLen = 0;
	}

	//将当前文本缓存到缓冲区
	assert(pStream->pText != NULL);
	if(nLen > 0)
	{
		memcpy(pStream->pText+pStream->nTextLen, sText, nLen);
		pStream->nTextLen += nLen;
	}

	return WDET_OK;
}

WDETcode CWordFind::nextTextFile(TextStream* h, const char* sTxtFile)
{
	char* pTxtBuf = NULL;
	int nSize = loadFile(sTxtFile, pTxtBuf);
	if(nSize < 0) return WDET_FILE_READ_FAILED;
	WDETcode rt = nextText(h, pTxtBuf, nSize);
	free(pTxtBuf);
	return rt;
}

WDETcode CWordFind::endIteration(TextStream* h, WORD_ARRAY &wordArray)
{
	WDETcode ret = WDET_OK;
	struct stTextStream *pStream = (struct stTextStream*)h;
	if(pStream == NULL) return WDET_BAD_FUNC_PARAM;

	//对未处理的文本进行处理
	if(pStream->nTextLen > 0)
	{
		WORD_ARRAY curWordArray;
		ret = extractWord(curWordArray, pStream->pText, pStream->nTextLen);
		mergeWordArray(pStream->vecWordContext, curWordArray);
		curWordArray.clear();
	}

	//输出到wordArray
	WORD_ARRAY::iterator it = pStream->vecWordContext.begin();
	for(; it!=pStream->vecWordContext.end(); it++)
	{
		wordArray.push_back(*it);
	}

	//释放pStream的内存空间
	free(pStream->pText);
	delete pStream;

	return ret;
}

WDETcode CWordFind::extractWord(WORD_ARRAY &wordArray, const char* sText, int nLen)
{
	//检查输入参数合法性
	if(sText == NULL || nLen < 0) return WDET_BAD_FUNC_PARAM;

	//这里分批进行处理以防止sText太大（每批最多处理MAX_TEXT_LENGTH个字节）
	int nOffset = 0;
	while(nOffset < nLen)
	{
		//将文本转换汉字/字符的内码序列 -> chInfoVec
		vector<int> vecCharCode;
		text2CharCodeVec(sText, nLen, nOffset, vecCharCode);

		//初始化树组pCharTreeA
		tCharTreeNode*	pCharTreeA = (tCharTreeNode*)calloc(WDET_MAX_CHARSET_SIZE, sizeof(tCharTreeNode));
		if(pCharTreeA == NULL) return WDET_OUT_OF_MEMORY;
		memcpy(pCharTreeA, m_pCharTreeA0, sizeof(tCharTreeNode)*WDET_MAX_CHARSET_SIZE);

		//根据内码序列创建树组 -> pCharTreeA
		buildCharTreeA(vecCharCode, pCharTreeA);

		//根据树组pCharTreeA导出所有词条及其相关信息 -> wordArray
		WORD_ARRAY curWordArray;
		exportAllWordsFromTreeA(pCharTreeA, curWordArray);

		//合并词条
		mergeWordArray(wordArray, curWordArray);

		//清空树组m_pCharTreeA
		for(unsigned int i = 0; i < WDET_MAX_CHARSET_SIZE; i++)
		{
			freeTree(pCharTreeA[i]);
		}
		free(pCharTreeA);
	}

	return WDET_OK;
}

WDETcode CWordFind::extractWord(WORD_ARRAY &wordArray, const char* sTxtFile)
{
	char* pTxtBuf = NULL;
	int nSize = loadFile(sTxtFile, pTxtBuf);
	if(nSize < 0) return WDET_FILE_READ_FAILED;
	WDETcode rt = extractWord(wordArray, pTxtBuf, nSize);
	free(pTxtBuf);
	return rt;
}

bool CWordFind::buildCharTreeA(const vector<int> & vecCharCode, tCharTreeNode *pCharTreeA)
{
	const int _BOUNDARY_CHAR_CODE = (int)(-1); //句子边界的内码
	const int _INVALID_CHAR_CODE = (int)(-2);
	unsigned int nPos = 0;
	int nLeftCharCode = _BOUNDARY_CHAR_CODE; //左临字的内码
	while(nPos < vecCharCode.size())
	{
		int ch1 = vecCharCode[nPos++]; //当前字
		pCharTreeA[ch1].nFreq++; //记录当前字的频度

		//字母或数字
		bool bAlphabetOrNumber = ALPHABET_OR_NUMBER(ch1);
		pCharTreeA[ch1].bAlphabetOrNumber = bAlphabetOrNumber;

		if(m_pFilter && m_pFilter->beMatched(ch1, BE_STOP_CHAR_FILTERED))
		{
			nLeftCharCode = ch1;
			continue;
		}

		//处理后续的(m_nMaxWordLen-1)个字或连续的字母数字串
		int nPosTemp = nPos;
		int nWordLen = 0;
		tCharTreeNode *pCurNode = &pCharTreeA[ch1];
		while(nPosTemp < (int)vecCharCode.size())
		{
			int ch2 = vecCharCode[nPosTemp++];

			//之前是连续的字母数字但ch2不是
			if(bAlphabetOrNumber && !ALPHABET_OR_NUMBER(ch2))
			{
				pCurNode->bValidString = 1;
				bAlphabetOrNumber = 0;
			}

			//判断该字是否在右邻字表中
			int iRightCharIndex = locateRightCharIndexByCode(*pCurNode,ch2);
			if(iRightCharIndex < 0)
			{
				//未找到则创建新的子节点
				iRightCharIndex = pCurNode->nRightCharCount; //新的子节点的索引值
				pCurNode->nRightCharCount++; //子节点个数
				pCurNode->pRightCharNodes = (tCharTreeNode*)realloc(pCurNode->pRightCharNodes, pCurNode->nRightCharCount*sizeof(tCharTreeNode));
				pCurNode->pRightCharNodes[iRightCharIndex].nRightCharCount = 0;
				pCurNode->pRightCharNodes[iRightCharIndex].nFreq = 0;
				pCurNode->pRightCharNodes[iRightCharIndex].chCode = ch2;
				pCurNode->pRightCharNodes[iRightCharIndex].pRightCharNodes = NULL;
				pCurNode->pRightCharNodes[iRightCharIndex].nLeftCharCount = 0;
				pCurNode->pRightCharNodes[iRightCharIndex].pLeftCharNodes = NULL;
				pCurNode->pRightCharNodes[iRightCharIndex].bAlphabetOrNumber = bAlphabetOrNumber;
				pCurNode->pRightCharNodes[iRightCharIndex].bValidString = 0;
			}
			pCurNode->pRightCharNodes[iRightCharIndex].nFreq++; //记录频度

			 //停用字
			if(m_pFilter && m_pFilter->beMatched(ch2, BE_STOP_CHAR_FILTERED)) break;

			//对左临字的处理
			pCurNode = &pCurNode->pRightCharNodes[iRightCharIndex]; //指向当前字对应的节点
			int iLeftCharIndex = locateLeftCharIndexByCode(*pCurNode, nLeftCharCode);
			if (iLeftCharIndex < 0)
			{
				iLeftCharIndex = pCurNode->nLeftCharCount;
				pCurNode->nLeftCharCount++;
				pCurNode->pLeftCharNodes = (tCharTreeNode*)realloc(pCurNode->pLeftCharNodes, pCurNode->nLeftCharCount*sizeof(tCharTreeNode));
				pCurNode->pLeftCharNodes[iLeftCharIndex].chCode = nLeftCharCode;
				pCurNode->pLeftCharNodes[iLeftCharIndex].nRightCharCount = 0;
				pCurNode->pLeftCharNodes[iLeftCharIndex].pRightCharNodes = NULL;
				pCurNode->pLeftCharNodes[iLeftCharIndex].nFreq = 0;
			}
			pCurNode->pLeftCharNodes[iLeftCharIndex].nFreq++;

			//跳过连续字母数字
			if(bAlphabetOrNumber)
			{
				nPos = nPosTemp;
				continue;
			}

			//已处理的词长加1
			nWordLen++;
			if(nWordLen >= m_nMaxWordLen) break;
		}
		nLeftCharCode = ch1; //记录左临字
	}
	return true;
}

void CWordFind::text2CharCodeVec(const char* sText, int nTextLen, int& nOffset, vector<int> & vecCharCode)
{
	if(sText == NULL || nTextLen == 0) return ;
	if(nOffset < 0 || nOffset >= nTextLen) return ; 
	vecCharCode.clear();

	//从偏移量nOffset开始将最多MAX_TEXT_LENGTH字节的文本转换成内码序列
	int nNextCharPos = nOffset;
	while(nNextCharPos < nTextLen)
	{
		int nChCode = getNextCharCode(sText, nTextLen, m_nCharset, nNextCharPos);
		if(nChCode == -1) continue;
		if(nChCode == 0x0d || nChCode == 0x0a) continue;
		if(m_nCharset == WDET_CHARSET_GBK && (nChCode == 0xA1B0 || nChCode == 0xA1B1)) continue; //双引号
		if(m_nCharset == WDET_CHARSET_GBK && (nChCode == 0xA1AE || nChCode == 0xA1AF)) continue; //单引号
		if(m_nCharset == WDET_CHARSET_UTF8 && (nChCode == 0x201C || nChCode == 0x201D)) continue; //双引号
		if(m_nCharset == WDET_CHARSET_UTF8 && (nChCode == 0x2018 || nChCode == 0x2019)) continue; //单引号
		//printf("%04xH\n", nChCode);

		vecCharCode.push_back(nChCode);
		if(nNextCharPos - nOffset >= MAX_TEXT_LENGTH) break;
	}

	//记录下次处理的偏移量
	nOffset = nNextCharPos;
}

void CWordFind::mergeWordArray(WORD_ARRAY & destWordArray, WORD_ARRAY & srcWordArray)
{
	sort(destWordArray.begin(), destWordArray.end(), wordGreaterByString);
	unsigned int nOldLen = (unsigned int)destWordArray.size();

	//将srcWordArray中所有词插入destWordArray中
	WORD_ARRAY::iterator it = srcWordArray.begin();
	for(; it != srcWordArray.end(); it++) //每次处理一个新词
	{
		//查找词表中是否已存在当前word
		WORD_ARRAY::iterator wit = lower_bound(destWordArray.begin(), destWordArray.begin()+nOldLen, (*it), wordGreaterByString);
		if(wit == destWordArray.begin()+nOldLen || (*wit)->sWord != (*it)->sWord)
		{
			//当前word为新出现的词
			destWordArray.push_back((*it));
			continue;
		}

		//当前word不是新出现的词

		(*wit)->nFreq += (*it)->nFreq;

		//将当前word此次获得的新的左临信息并入到该word以前的信息中
		for(int k = 0; k < (*it)->nLeftCharCount; k++)
		{
			bool bNewLeftChar = true;
			for(int j = 0; j < (*wit)->nLeftCharCount; j++)
			{
				if((*it)->pLeftChars[k] == (*wit)->pLeftChars[j])
				{
					(*wit)->pnLeftFreqs[j] += (*it)->pnLeftFreqs[k];
					bNewLeftChar = false;
					break;
				}
			}
			if(bNewLeftChar)
			{
				(*wit)->pLeftChars = (int*)realloc((*wit)->pLeftChars, ((*wit)->nLeftCharCount+1)*sizeof(int));
				(*wit)->pLeftChars[(*wit)->nLeftCharCount] = (*it)->pLeftChars[k];
				(*wit)->pnLeftFreqs = (unsigned int*)realloc((*wit)->pnLeftFreqs, ((*wit)->nLeftCharCount+1)*sizeof(unsigned int));
				(*wit)->pnLeftFreqs[(*wit)->nLeftCharCount] = (*it)->pnLeftFreqs[k];
				(*wit)->nLeftCharCount++;
			}
		}
		
		//将当前word此次获得的新的右临信息并入到该word以前的信息中
		for(int k = 0; k < (*it)->nRightCharCount; k++)
		{
			bool bNewRightChar = true;
			for(int j = 0; j < (*wit)->nRightCharCount; j++)
			{
				if((*it)->pRightChars[k] == (*wit)->pRightChars[j])
				{
					(*wit)->pnRightFreqs[j] += (*it)->pnRightFreqs[k];
					bNewRightChar = false;
					break;
				}
			}
			if(bNewRightChar)
			{
				(*wit)->pRightChars = (int*)realloc((*wit)->pRightChars, ((*wit)->nRightCharCount+1)*sizeof(int));
				(*wit)->pRightChars[(*wit)->nRightCharCount] = (*it)->pRightChars[k];
				(*wit)->pnRightFreqs = (unsigned int*)realloc((*wit)->pnRightFreqs, ((*wit)->nRightCharCount+1)*sizeof(unsigned int));
				(*wit)->pnRightFreqs[(*wit)->nRightCharCount] = (*it)->pnRightFreqs[k];
				(*wit)->nRightCharCount++;
			}
		}

		free((*it)->pLeftChars);
		free((*it)->pnLeftFreqs);
		free((*it)->pRightChars);
		free((*it)->pnRightFreqs);
		delete (*it);
	}
}

bool CWordFind::exportAllWordsFromTreeA(const tCharTreeNode *pCharTreeA, WORD_ARRAY & wordArray)
{
	wordArray.clear();
	for(unsigned int i=0; i<WDET_MAX_CHARSET_SIZE; i++)
	{
		const tCharTreeNode *pCurNode = &pCharTreeA[i];

		vector<int> codePath;
		codePath.push_back(i);

		exportWord(pCurNode, codePath, wordArray);
		codePath.clear();
	}	
	return true;
}

void CWordFind::exportWord(const tCharTreeNode *pTreeNode, vector<int>& codePath, WORD_ARRAY & wordArray)
{
	if (pTreeNode == NULL) return ;
	if(min(pTreeNode->nRightCharCount, pTreeNode->nLeftCharCount) >= m_nAVThresh || pTreeNode->bValidString)
	{
		int nFirstChar = codePath[0];
		int nFinalChar = codePath[codePath.size()-1];

		//凡是首字匹配过滤规则的串都不导出
		if(m_pFilter!=NULL && m_pFilter->beMatched(nFirstChar, BE_FIRST_CHAR_FILTERED)) return ;

		//凡是尾字匹配过滤规则的串都不导出
		if(m_pFilter!=NULL && m_pFilter->beMatched(nFinalChar, BE_FINAL_CHAR_FILTERED)) goto __NEXT__;

		//拼接一个词条 -> sWord
		string sWord;
		int nCharCode = 0;
		vector<int>::iterator it = codePath.begin();
		for(unsigned int i=0; i < codePath.size(); i++)
		{
			nCharCode = codePath[i];

			//凡是含有停用字的串都不导出
			if(m_pFilter && m_pFilter->beMatched(nCharCode, BE_STOP_CHAR_FILTERED)) return ; 

			//输出当前汉字/字符
			sWord += getCharByCode(nCharCode, m_nCharset); 
		}

		//词长不大于2个字节的都不导出
		if(sWord.size() <= 2) goto __NEXT__;
	
		//凡是满足整词过滤规则的都不导出
		//printf("*****%s\n", sWord.c_str());
		if (pTreeNode->bValidString && pTreeNode->bAlphabetOrNumber)
		{
			if(m_pFilter && m_pFilter->beEnStopWord(sWord.c_str())) return ;
		}
		else
		{
			if(m_pFilter && m_pFilter->beMatched(sWord.c_str(), BE_STOP_CNWORD_FILTERED)) return ;
		}

		//为用户的旧词都不导出
		if(m_pfOldWord!=NULL && m_pfOldWord(sWord.c_str())) goto __NEXT__;
			
		//导出sWord
		{
			unsigned short nLeftCharCount = pTreeNode->nLeftCharCount;
			unsigned short nRightCharCount = pTreeNode->nRightCharCount;
			
			//获得当前word的前缀字数组 -> pLeftChars
			int *pLeftChars = (int *)calloc(nLeftCharCount,sizeof(int));
			unsigned int *pnLeftFreqs = (unsigned int*)calloc(nLeftCharCount,sizeof(unsigned int));
			assert(pLeftChars != NULL && pnLeftFreqs != NULL);
			for(int k = 0; k < nLeftCharCount; k++)
			{
				pLeftChars[k] = pTreeNode->pLeftCharNodes[k].chCode;
				pnLeftFreqs[k] = pTreeNode->pLeftCharNodes[k].nFreq;
			}
			
			//获得当前word的后缀字数组 -> pRightChars
			int *pRightChars = (int *)calloc(nRightCharCount,sizeof(int));
			unsigned int *pnRightFreqs = (unsigned int*)calloc(nRightCharCount,sizeof(unsigned int));
			assert(pRightChars != NULL && pnRightFreqs != NULL);
			for(int k = 0; k < nRightCharCount; k++)
			{
				pRightChars[k] = pTreeNode->pRightCharNodes[k].chCode;
				pnRightFreqs[k] = pTreeNode->pRightCharNodes[k].nFreq;
			}

			//当前word
			//printf("%s\n", sWord.c_str());
			CWordContext *w = new CWordContext();
			assert(w != NULL);
			w->sWord = sWord;
			w->dbFirstProb = (m_pFilter==NULL ? 1.0 : m_pFilter->firstProb(nFirstChar));
			w->dbFinalProb = (m_pFilter==NULL ? 1.0 : m_pFilter->finalProb(nFinalChar));
			w->nLeftCharCount = nLeftCharCount;
			w->nRightCharCount = nRightCharCount;
			w->nFreq = pTreeNode->nFreq;
			w->pLeftChars = pLeftChars;
			w->pRightChars = pRightChars;
			w->pnLeftFreqs = pnLeftFreqs;
			w->pnRightFreqs = pnRightFreqs;
			w->dbScore = 0.0;
			wordArray.push_back(w);
		}
	}

__NEXT__:

	//遍历所有孩子
	for(unsigned int i=0; i<pTreeNode->nRightCharCount; i++)
	{
		if(pTreeNode->pRightCharNodes[i].nFreq >= 2)
		{
			codePath.push_back(pTreeNode->pRightCharNodes[i].chCode);
			exportWord(&pTreeNode->pRightCharNodes[i], codePath, wordArray);
			codePath.pop_back();
		}
	}
}

int CWordFind::locateRightCharIndexByCode(const tCharTreeNode &treeNode, int nCharCode)
{
	for (unsigned int i = 0; i < treeNode.nRightCharCount; i++)
	{
		if (treeNode.pRightCharNodes[i].chCode == nCharCode) return i;
	}
	return -1;
}

int CWordFind::locateLeftCharIndexByCode(const tCharTreeNode &treeNode, int nCharCode)
{
	for (unsigned int i = 0; i < treeNode.nLeftCharCount; i++)
	{
		if (treeNode.pLeftCharNodes[i].chCode == nCharCode) return i;
	}
	return -1;
}

void CWordFind::freeTree(tCharTreeNode &rootNode)
{
	if(rootNode.pLeftCharNodes != NULL)
	{
		free(rootNode.pLeftCharNodes);
		rootNode.pLeftCharNodes = NULL;
		rootNode.nLeftCharCount = 0;
	}
	if (rootNode.pRightCharNodes != NULL)
	{
		for (unsigned int i=0; i<rootNode.nRightCharCount; i++)
		{
			freeTree(rootNode.pRightCharNodes[i]);
		}
		free(rootNode.pRightCharNodes);
		rootNode.pRightCharNodes = NULL;
		rootNode.nRightCharCount = 0;
	}
}

}
