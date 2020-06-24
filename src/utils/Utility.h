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

//读取文件
int loadFile(const char *sFilename, char* & pBuffer);

//获得下一个字符/汉字的内码
int getNextCharCode(const char* sText, int nTextLen, WDETcharset cs, int & nNextCharPos);

//根据内码还原成原始字符/汉字
string getCharByCode(int nCharCode, WDETcharset cs);

//得到长为n的随机串
string GenRandomString(int n);

bool isBigEndian();
unsigned long long htonll(unsigned long long x);

//缓冲区加密解密
bool encrypt_buffer(char* pData, unsigned int nDataLen, const char* sKey, unsigned int nKeyLen);

//文件加密
bool encrypt_file(const char* from_file, const char* to_file);

//文件夹加密
bool encrypt_dir(const char* from_dir, const char* to_dir);

//载入加密文件，输出解密结果
int loadEncryptFile(const char* file, char* & pBuffer);

//路径规范化
void NormalizePath(char* dir, bool bTail=true);
void NormalizePath(string& dir,  bool bTail=true);

//创建文件路径
bool CreateFilePath(const char* path);

//读取文本缓冲区的每一行存入字符串数组
void parseTextByLine(const char* text, int size, vector<string>& result);

//转换为小写
void toLower(string & s);

}

#endif //_H_UTILITY_GDDING_INCLUDED_20101201

