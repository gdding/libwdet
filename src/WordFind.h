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
 	 * ��������: ���ı��г�ȡ����
 	 * �������: sText           - Դ�ı�
 	 *            nLen            - Դ�ı����ȣ��ֽ�����
	 * �������: wordArray       - ��������
	 * ��ע����: (1) �����ı��ĳ��Ȳ��ܳ�������ֵ��MAX_TEXT_LENGTH
	 *            (2) wordArray�е�����ָ��������߾�����ʱ�ͷ�
	 **************************************************************/
	WDETcode extractWord(WORD_ARRAY &wordArray, const char* sText, int nLen);
	WDETcode extractWord(WORD_ARRAY &wordArray, const char* sTxtFile);

	//���ı����Ĵ���
	TextStream* startIteration();
	WDETcode nextText(TextStream* h, const char* sText, int nLen);
	WDETcode nextTextFile(TextStream* h, const char* sTxtFile);
	WDETcode endIteration(TextStream* h, WORD_ARRAY &wordArray);

	void FreeWordArray(WORD_ARRAY &wordArray);

protected:
	//���ڵ�ṹ�嶨��
	typedef struct stCharTreeNode
	{				
		int							chCode;				//��ǰ�ڵ�ֵ��������룩
		unsigned int				nFreq;				//Ƶ��
		unsigned short				nRightCharCount;	//��ͬ�����ֵ���Ŀ
		struct stCharTreeNode*		pRightCharNodes;	//�����ֽڵ�����
		unsigned short				nLeftCharCount;		//��ͬ�����ֵ���Ŀ
		struct stCharTreeNode*		pLeftCharNodes;		//�����ֽڵ�����
		unsigned char				bAlphabetOrNumber;	//������ĸ���֣�
		unsigned char				bValidString;		//��Ч����ĸ���ִ�
	}tCharTreeNode;

protected:
	//���ı�ת������/�ַ����������� -> chInfoVec
	void text2CharCodeVec(const char* sText, int nTextLen, int& nOffset, vector<int> & vecCharCode);

	//�����������д������� -> pCharTreeA
	bool buildCharTreeA(const vector<int> & vecCharCode, tCharTreeNode *pCharTreeA);

	//��������pCharTreeA�������д������������Ϣ -> wordArray
	bool exportAllWordsFromTreeA(const tCharTreeNode* pCharTreeA, WORD_ARRAY &wordArray);
	
	//�����ϵĸ����ڵ㿪ʼ��������
	void exportWord(const tCharTreeNode *pTreeNode,
					vector<int>& codePath, 
					WORD_ARRAY & wordArray);

	//�ϲ�������������: srcWordArray + destWordArray -> destWordArray
	void mergeWordArray(WORD_ARRAY & destWordArray, WORD_ARRAY & srcWordArray);

	//��λ��ǰ�ֽڵ���ĸ�������Ϊָ������
	int locateRightCharIndexByCode(const tCharTreeNode &treeNode, int nCharCode);

	//��λ��ǰ�ֽڵ���ĸ�������Ϊָ������
	int locateLeftCharIndexByCode(const tCharTreeNode &treeNode, int nCharCode);

	//�ͷ�һ����
	void freeTree(tCharTreeNode &rootNode);

protected:
	WDETcharset		m_nCharset;			//ÿ�δ�����ı��ַ���
	int				m_nMaxTextLen;		//ÿ�δ��������ı����ȣ��ֽ�����
	int				m_nMaxWordLen;		//���ʳ���������
	int				m_nAVThresh;		//AV��ֵ
	CFilter*		m_pFilter;			//����������
	tCharTreeNode*	m_pCharTreeA0;
	cb_existed_word m_pfOldWord;		//�ص�����

private: //�������ݽṹֻ���ڴ����ı��������
	struct stTextStream
	{
		char*			pText;			//���ܵĴ������ı�
		int				nTextLen;		//�ı�����
		WORD_ARRAY		vecWordContext;	//������word�������ͳ����Ϣ
	};
};


class CWordContext
{
public:
	string			sWord;				//�ʵ��ַ���
	double			dbFirstProb;		//���ֳɴʸ���
	double			dbFinalProb;		//β�ֳɴʸ���
	unsigned int	nFreq;				//�ôʵĳ���Ƶ��
	double			dbScore;			//����ֵ
	unsigned short	nLeftCharCount;		//�ôʵ������ֵ���Ŀ����pLeftChars�е�Ԫ�ظ���
	unsigned short	nRightCharCount;	//�ôʵ������ֵ���Ŀ����pRightChars�е�Ԫ�ظ���
	int*			pLeftChars;			//�ôʵ������ֵ���������
	unsigned int*	pnLeftFreqs;		//�ôʵ������ֵ�Ƶ������
	int*			pRightChars;		//�ôʵ������ֵ���������
	unsigned int*	pnRightFreqs;		//�ôʵ������ֵ�Ƶ������
};
}

#endif //_H_WORDFIND_GDDING_INCLUDED_20071210
