/*---------------------------------------------------------------
* Copyright (c) 2007~2010 Ding Guodong. All rights reserved.
*   dingguodong@software.ict.ac.cn; gdding@hotmail.com
*
* This file is the confidential and proprietary property of 
* Ding Guodong and the posession or use of this file requires 
* a written license from the author.

* Last-Modified: 2010-12-01
*---------------------------------------------------------------*/
#ifndef _H_FILTER_GDDING_INCLUDED_20101201
#define _H_FILTER_GDDING_INCLUDED_20101201
#include "utils/StdHeader.h"
#include "wdet.h"
#include "utils/Utility.h"


namespace ns_wdet
{

typedef enum
{
	BE_INVALID_CHAR			= 0, //非法字符
	BE_STOP_CHAR_FILTERED	= 1, //停用字符
	BE_FIRST_CHAR_FILTERED	= 2, //需过滤掉的首字
	BE_FINAL_CHAR_FILTERED	= 3, //需过滤掉的尾字
	BE_STOP_CNWORD_FILTERED	= 4, //需过滤掉的中文子词
	BE_SURNAME_CHAR			= 5,
	BE_STOP_ENWORD_FILTERED	= 6 //需过滤掉的英文单词
}FilterRule;

class CFilter
{
public:
	CFilter(WDETcharset cs);
	virtual ~CFilter();

public:
	WDETcode setCharset(WDETcharset cs);
	WDETcode setStopCharFilterFile(const char* filename);
	WDETcode setCNStopWordFilterFile(const char* filename);
	WDETcode setENStopWordFilterFile(const char* filename);
	WDETcode setFirstCharFilterFile(const char* filename);
	WDETcode setFinalCharFilterFile(const char* filename);
	WDETcode setCharProbFile(const char* filename);

public:
	//判断一个字符是否匹配一条过滤规则
	inline bool beMatched(int code, FilterRule rule);

	//判断一个词条是否匹配一条过滤规则
	inline bool beMatched(const char* sWord, FilterRule rule);

	//英文停用词
	inline bool beEnStopWord(const char* sWord);

public:
	//得到一个字符为首字的概率
	inline double firstProb(int code);

	//得到一个字符为尾字的概率
	inline double finalProb(int code);

protected:
	WDETcode setFlag(const char* sFilterFile, char* pCharFilter);
	WDETcode readFilterCharFile(const char* sFilterCharFile, bool* & pbCharVec);

protected:
	//字的统计信息
	typedef struct 
	{
		int		chCode;			//字的内码
		double	dbFirstProb;	//该字是首字的概率
		double	dbFinalProb;	//该字是尾字的概率
	}tCharStatInfo;

	char*				m_pStopCharFilter;
	char*				m_pFirstCharFilter;
	char*				m_pFinalCharFilter;
	set<string>			m_filterWordSet;	//需过滤掉的中文词条
	set<string>			m_filterWordSetEn;	//需过滤掉的英文词条
	
	tCharStatInfo*	m_pCharStatInfo;

private:
	WDETcharset	m_nCharset; //每次处理的文本字符集
};

//判断一个字符是否匹配一条规则
inline bool CFilter::beMatched(int code, FilterRule rule)
{
	switch(rule)
	{
	case BE_STOP_CHAR_FILTERED:
		return (m_pStopCharFilter!=NULL && m_pStopCharFilter[code]==0x01);
	case BE_FIRST_CHAR_FILTERED:
		return (m_pFirstCharFilter!=NULL && m_pFirstCharFilter[code]==0x01);
	case BE_FINAL_CHAR_FILTERED:
		return (m_pFinalCharFilter!=NULL && m_pFinalCharFilter[code]==0x01);
	case BE_SURNAME_CHAR:
		return false;
	case BE_INVALID_CHAR:
		return false;
	default:
		return false;
	};
}

inline bool CFilter::beMatched(const char* sWord, FilterRule rule)
{
	if(rule != BE_STOP_CNWORD_FILTERED) return false;
	set<string>::iterator it = m_filterWordSet.begin();
	for(; it!=m_filterWordSet.end(); it++)
	{
		if((string(sWord)).find(*it) != string::npos) return true;
	}
	return false;
}

inline bool CFilter::beEnStopWord(const char* sWord)
{
	string sLowerWord(sWord);
	toLower(sLowerWord);
	set<string>::iterator it = m_filterWordSetEn.begin();
	for(; it!=m_filterWordSetEn.end(); it++)
	{
		if((*it) == sWord) return true;
		if((*it) == sLowerWord) return true;
	}
	return false;
}

//得到一个字符为首字的概率
inline double CFilter::firstProb(int code)
{
	assert(code >= 0 && code < WDET_MAX_CHARSET_SIZE);
	return m_pCharStatInfo[code].dbFirstProb;
}

//得到一个字符为尾字的概率
inline double CFilter::finalProb(int code)
{
	assert(code >= 0 && code < WDET_MAX_CHARSET_SIZE);
	return m_pCharStatInfo[code].dbFinalProb;
}

}

#endif //_H_FILTER_GDDING_INCLUDED_20101201
