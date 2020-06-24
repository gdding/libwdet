/*---------------------------------------------------------------
* Copyright (c) 2007~2010 Ding Guodong. All rights reserved.
*   dingguodong@software.ict.ac.cn; gdding@hotmail.com
*
* This file is the confidential and proprietary property of 
* Ding Guodong and the posession or use of this file requires 
* a written license from the author.

* Last-Modified: 2010-12-04
*---------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "wdet.h"

#ifdef WIN32
	#pragma comment(lib, "libwdet")
#endif

/**
 * ��������ʾ��WDET����Ҫ�ӿ�ʹ�÷���
 * ��������ԶԸ������ı��ļ������´�/�����崮����
 * ��������ʵĴʻ㡢Ƶ�ȡ���������������������
 **/

bool is_existed_word(const char* word)
{
	//��������Ա�д�Լ��Ĵ��������ж�һ�����Ƿ����Ѵ��ڵĴ�
	//�Ѵ��ڵĴʾͲ���������´ʷ��ֽ������
	//if(strcmp(word, "�й�") == 0) return true;
	//if(strcmp(word, "�л�") == 0) return true;
	return false;
}

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		printf("usage: %s <text_file> <charset>\n", argv[0]);
		printf("where:\n    charset - charset of the text file, 0 for gbk, 1 for utf-8\n");
		return 0;
	}
	const char* text_file = argv[1];
	int charset = atoi(argv[2]);

	//��ʼ�������ò���
	WDET* h = wdet_init();
	wdet_setopt(h, WDETOPT_CHARSET, charset);
	wdet_setopt(h, WDETOPT_MIN_AVTHRESH, 2);
	wdet_setopt(h, WDETOPT_WORD_EXISTED_FUNC, is_existed_word);

	//�ӱ����ļ�����ȡ�´�/�����崮
	time_t tBeginTime = time(0);
	int count = 0;
	wdet_word* result = wdet_perform_file(h, text_file, &count);
	time_t tEndTime = time(0);
	if(count == 0)
	{
		printf("δ��ȡ�����.\n");
	}

	//��ʾ���
	for(int i = 0; i < count; i++)
	{
		fprintf(stdout, "%-20s %-4d %-4d %-4d %.9f\n", 
				result[i].word,
				result[i].freq, 
				result[i].leftcount, 
				result[i].rightcount, 
				result[i].score);
	}
	printf("Time used: %d seconds\n", (int)(tEndTime - tBeginTime));

	//�ͷŽ��
	wdet_free_result(result);

	//���پ��
	wdet_cleanup(h);

	return 0;
}

