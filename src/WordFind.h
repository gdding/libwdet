/*---------------------------------------------------------------
* Copyright (c) 2007~2010 Ding Guodong. All rights reserved.
*   dingguodong@software.ict.ac.cn; gdding@hotmail.com
*
* This file is the confidential and proprietary property of 
* Ding Guodong and the posession or use of this file requires 
* a written license from the author.

* Last-Modified: 2010-10-30
*---------------------------------------------------------------*/
#ifndef _H_WORDFIND_GDDING_INCLUDED_20071210
#define _H_WORDFIND_GDDING_INCLUDED_20071210
#include "utils/StdHeader.h"
#include "wdet.h"

namespace ns_wdet
{

class CFilter;
class CWordContext;
typedef vector<CWordContext*> WORD_ARRAY;
typedef void TextStream;

class CWordFind
{
public:
	CWordFind();
	virtual ~CWordFind();

public:
	WDETcode SetOption(WDETOption opt, va_list param);

	/**************************************************************
 	 * 函数功能: 从文本中抽取词条
 	 * 输入参数: sText           - 源文本
 	 *            nLen            - 源文本长度（字节数）
	 * 输出参数: wordArray       - 词条数组
	 * 备注事项: (1) 输入文本的长度不能超过上限值即MAX_TEXT_LENGTH
	 *            (2) wordArray中的两个指针需调用者决定何时释放
	 **************************************************************/
	WDETcode extractWord(WORD_ARRAY &wordArray, const char* sText, int nLen);
	WDETcode extractWord(WORD_ARRAY &wordArray, const char* sTxtFile);

	//对文本流的处理
	TextStream* startIteration();
	WDETcode nextText(TextStream* h, const char* sText, int nLen);
	WDETcode nextTextFile(TextStream* h, const char* sTxtFile);
	WDETcode endIteration(TextStream* h, WORD_ARRAY &wordArray);

	void FreeWordArray(WORD_ARRAY &wordArray);

protected:
	//树节点结构体定义
	typedef struct stCharTreeNode
	{				
		int							chCode;				//当前节点值（存放内码）
		unsigned int				nFreq;				//频度
		unsigned short				nRightCharCount;	//不同右临字的数目
		struct stCharTreeNode*		pRightCharNodes;	//右临字节点数组
		unsigned short				nLeftCharCount;		//不同左临字的数目
		struct stCharTreeNode*		pLeftCharNodes;		//左临字节点数组
		unsigned char				bAlphabetOrNumber;	//连续字母数字？
		unsigned char				bValidString;		//有效的字母数字串
	}tCharTreeNode;

protected:
	//将文本转换汉字/字符的内码序列 -> chInfoVec
	void text2CharCodeVec(const char* sText, int nTextLen, int& nOffset, vector<int> & vecCharCode);

	//根据内码序列创建树组 -> pCharTreeA
	bool buildCharTreeA(const vector<int> & vecCharCode, tCharTreeNode *pCharTreeA);

	//根据树组pCharTreeA导出所有词条及其相关信息 -> wordArray
	bool exportAllWordsFromTreeA(const tCharTreeNode* pCharTreeA, WORD_ARRAY &wordArray);
	
	//从树上的给定节点开始导出词条
	void exportWord(const tCharTreeNode *pTreeNode,
					vector<int>& codePath, 
					WORD_ARRAY & wordArray);

	//合并两个词条数组: srcWordArray + destWordArray -> destWordArray
	void mergeWordArray(WORD_ARRAY & destWordArray, WORD_ARRAY & srcWordArray);

	//定位当前字节点的哪个右临字为指定的字
	int locateRightCharIndexByCode(const tCharTreeNode &treeNode, int nCharCode);

	//定位当前字节点的哪个左临字为指定的字
	int locateLeftCharIndexByCode(const tCharTreeNode &treeNode, int nCharCode);

	//释放一棵树
	void freeTree(tCharTreeNode &rootNode);

protected:
	WDETcharset		m_nCharset;			//每次处理的文本字符集
	int				m_nMaxTextLen;		//每次处理的最大文本长度（字节数）
	int				m_nMaxWordLen;		//最大词长（字数）
	int				m_nAVThresh;		//AV阈值
	CFilter*		m_pFilter;			//词条过滤器
	tCharTreeNode*	m_pCharTreeA0;
	cb_existed_word m_pfOldWord;		//回调函数

private: //以下数据结构只用于处理文本流的情况
	struct stTextStream
	{
		char*			pText;			//积攒的待处理文本
		int				nTextLen;		//文本长度
		WORD_ARRAY		vecWordContext;	//检测出的word及其相关统计信息
	};
};


class CWordContext
{
public:
	string			sWord;				//词的字符串
	double			dbFirstProb;		//首字成词概率
	double			dbFinalProb;		//尾字成词概率
	unsigned int	nFreq;				//该词的出现频度
	double			dbScore;			//评分值
	unsigned short	nLeftCharCount;		//该词的左临字的数目，即pLeftChars中的元素个数
	unsigned short	nRightCharCount;	//该词的右临字的数目，即pRightChars中的元素个数
	int*			pLeftChars;			//该词的左临字的内码数组
	unsigned int*	pnLeftFreqs;		//该词的左临字的频度数组
	int*			pRightChars;		//该词的右临字的内码数组
	unsigned int*	pnRightFreqs;		//该词的右临字的频度数组
};
}

#endif //_H_WORDFIND_GDDING_INCLUDED_20071210
