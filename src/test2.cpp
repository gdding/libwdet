/*---------------------------------------------------------------
* Copyright (c) 2007~2010 Ding Guodong. All rights reserved.
*   dingguodong@software.ict.ac.cn; gdding@hotmail.com
*
* This file is the confidential and proprietary property of 
* Ding Guodong and the posession or use of this file requires 
* a written license from the author.

* Last-Modified: 2010-12-04
*---------------------------------------------------------------*/
#ifndef WIN32
#include <dirent.h>
#endif
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <string.h>
using namespace std;
#include "wdet.h"
#include "opendir.h"


/**
 * ��������ʾ��WDET���ı�����ʽ����Ҫ�ӿ�ʹ�÷���
 * ��������ԶԸ������ļ��н����´�/�����崮����
 * ��������ʵĴʻ㡢Ƶ�ȡ���������������������
 **/
#ifdef WIN32
	#pragma comment(lib, "libwdet")
#endif

bool is_existed_word(const char* word)
{
	//��������Ա�д�Լ��Ĵ��������ж�һ�����Ƿ����Ѵ��ڵĴ�
	//�Ѵ��ڵĴʾͲ���������´ʷ��ֽ������

	return false;
}

void getTextFiles(const char* sPath, vector<string> &vecFile);

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		printf("Usage: %s <text_dir> <charset> <result_file>\n", argv[0]);
		printf("Where: charset - charset of the text directory, 0 for GBK, 1 for UTF-8\n");
		return 0;
	}
	const char* sTextDir = argv[1];
	int charset = atoi(argv[2]);
	const char* sResultFile = argv[3];
	
	//��ʼ�������ø������
	WDET* h = wdet_init();
	wdet_setopt(h, WDETOPT_CHARSET, charset);
	wdet_setopt(h, WDETOPT_WORD_EXISTED_FUNC, is_existed_word);

	//���ָ��Ŀ¼�µ��ı��ļ���
	vector<string> files;
	getTextFiles(sTextDir, files);

	//���ı�����ʽ�����´�/�����崮����
	time_t tBeginTime = time(0);
	WDETM* hm = wdet_stream_start(h);
	assert(hm != NULL);
	for(unsigned int i=0; i < files.size(); i++)
	{
		printf("performing: %s\n", files[i].c_str());
		wdet_stream_perform_file(h, hm, files[i].c_str());
	}
	int count = 0;
	wdet_word* result = wdet_stream_end(h, hm, &count);
	time_t tEndTime = time(0);
	if(count == 0)
	{
		printf("δ��ȡ�����.\n");
	}
	//��ʾ���
	FILE* fp = fopen(sResultFile, "wt");
	assert(fp != NULL);
	for(int i = 0; i < count; i++)
	{
		fprintf(fp, "%-20s %-4d %-4d %-4d %.9f\n", 
				result[i].word,
				result[i].freq, 
				result[i].leftcount, 
				result[i].rightcount, 
				result[i].score);
	}
	fclose(fp);
	printf("Time used: %d seconds\n", (int)(tEndTime - tBeginTime));
	
	//�ͷŽ��
	wdet_free_result(result);

	//���پ��
	wdet_cleanup(h);

	return 0;
}


void getTextFiles(const char* sPath, vector<string> &vecFile)
{
	DIR *dp = opendir(sPath);
	if(!dp) return ;
	string sCorrectPath(sPath);
	if(sCorrectPath[sCorrectPath.length()-1] != '/' && sCorrectPath[sCorrectPath.length()-1] != '\\')
		sCorrectPath += '/';

	dirent* dirp = NULL;
	while((dirp=readdir(dp)) != NULL)
	{
		if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0)
		{
			continue;
		}
		if(strstr(dirp->d_name,".txt"))
		{
			string sFile = sCorrectPath + dirp->d_name;
			vecFile.push_back(sFile);
		}
		else
		{
			string sChildPath = sCorrectPath + dirp->d_name;
			sChildPath += '/';
			getTextFiles(sChildPath.c_str(), vecFile);
		}
	}

	closedir(dp);
	return ;
}
