/*---------------------------------------------------------------
* Copyright (c) 2007~2010 Ding Guodong. All rights reserved.
*   dingguodong@software.ict.ac.cn; gdding@hotmail.com
*
* This file is the confidential and proprietary property of 
* Ding Guodong and the posession or use of this file requires 
* a written license from the author.

* Last-Modified: 2010-12-01
*---------------------------------------------------------------*/
#ifndef _H_UTILITY_GDDING_INCLUDED_20101201
#define _H_UTILITY_GDDING_INCLUDED_20101201
#include "StdHeader.h"
#include "../wdet.h"


namespace ns_wdet
{

//��ȡ�ļ�
int loadFile(const char *sFilename, char* & pBuffer);

//�����һ���ַ�/���ֵ�����
int getNextCharCode(const char* sText, int nTextLen, WDETcharset cs, int & nNextCharPos);

//�������뻹ԭ��ԭʼ�ַ�/����
string getCharByCode(int nCharCode, WDETcharset cs);

//�õ���Ϊn�������
string GenRandomString(int n);

bool isBigEndian();
unsigned long long htonll(unsigned long long x);

//���������ܽ���
bool encrypt_buffer(char* pData, unsigned int nDataLen, const char* sKey, unsigned int nKeyLen);

//�ļ�����
bool encrypt_file(const char* from_file, const char* to_file);

//�ļ��м���
bool encrypt_dir(const char* from_dir, const char* to_dir);

//��������ļ���������ܽ��
int loadEncryptFile(const char* file, char* & pBuffer);

//·���淶��
void NormalizePath(char* dir, bool bTail=true);
void NormalizePath(string& dir,  bool bTail=true);

//�����ļ�·��
bool CreateFilePath(const char* path);

//��ȡ�ı���������ÿһ�д����ַ�������
void parseTextByLine(const char* text, int size, vector<string>& result);

//ת��ΪСд
void toLower(string & s);

}

#endif //_H_UTILITY_GDDING_INCLUDED_20101201

