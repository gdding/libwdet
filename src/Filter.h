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
	BE_INVALID_CHAR			= 0, //�Ƿ��ַ�
	BE_STOP_CHAR_FILTERED	= 1, //ͣ���ַ�
	BE_FIRST_CHAR_FILTERED	= 2, //����˵�������
	BE_FINAL_CHAR_FILTERED	= 3, //����˵���β��
	BE_STOP_CNWORD_FILTERED	= 4, //����˵��������Ӵ�
	BE_SURNAME_CHAR			= 5,
	BE_STOP_ENWORD_FILTERED	= 6 //����˵���Ӣ�ĵ���
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
	//�ж�һ���ַ��Ƿ�ƥ��һ�����˹���
	inline bool beMatched(int code, FilterRule rule);

	//�ж�һ�������Ƿ�ƥ��һ�����˹���
	inline bool beMatched(const char* sWord, FilterRule rule);

	//Ӣ��ͣ�ô�
	inline bool beEnStopWord(const char* sWord);

public:
	//�õ�һ���ַ�Ϊ���ֵĸ���
	inline double firstProb(int code);

	//�õ�һ���ַ�Ϊβ�ֵĸ���
	inline double finalProb(int code);

protected:
	WDETcode setFlag(const char* sFilterFile, char* pCharFilter);
	WDETcode readFilterCharFile(const char* sFilterCharFile, bool* & pbCharVec);

protected:
	//�ֵ�ͳ����Ϣ
	typedef struct 
	{
		int		chCode;			//�ֵ�����
		double	dbFirstProb;	//���������ֵĸ���
		double	dbFinalProb;	//������β�ֵĸ���
	}tCharStatInfo;

	char*				m_pStopCharFilter;
	char*				m_pFirstCharFilter;
	char*				m_pFinalCharFilter;
	set<string>			m_filterWordSet;	//����˵������Ĵ���
	set<string>			m_filterWordSetEn;	//����˵���Ӣ�Ĵ���
	
	tCharStatInfo*	m_pCharStatInfo;

private:
	WDETcharset	m_nCharset; //ÿ�δ�����ı��ַ���
};

//�ж�һ���ַ��Ƿ�ƥ��һ������
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

//�õ�һ���ַ�Ϊ���ֵĸ���
inline double CFilter::firstProb(int code)
{
	assert(code >= 0 && code < WDET_MAX_CHARSET_SIZE);
	return m_pCharStatInfo[code].dbFirstProb;
}

//�õ�һ���ַ�Ϊβ�ֵĸ���
inline double CFilter::finalProb(int code)
{
	assert(code >= 0 && code < WDET_MAX_CHARSET_SIZE);
	return m_pCharStatInfo[code].dbFinalProb;
}

}

#endif //_H_FILTER_GDDING_INCLUDED_20101201
