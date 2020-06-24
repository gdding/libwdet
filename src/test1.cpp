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
 * 本程序演示了WDET的主要接口使用方法
 * 本程序可以对给定的文本文件进行新词/有意义串发现
 * 输出各个词的词汇、频度、左邻数、右邻数、评分
 **/

bool is_existed_word(const char* word)
{
	//这里你可以编写自己的代码用来判断一个词是否是已存在的词
	//已存在的词就不会出现在新词发现结果中了
	//if(strcmp(word, "中国") == 0) return true;
	//if(strcmp(word, "中华") == 0) return true;
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

	//初始化并设置参数
	WDET* h = wdet_init();
	wdet_setopt(h, WDETOPT_CHARSET, charset);
	wdet_setopt(h, WDETOPT_MIN_AVTHRESH, 2);
	wdet_setopt(h, WDETOPT_WORD_EXISTED_FUNC, is_existed_word);

	//从本文文件中提取新词/有意义串
	time_t tBeginTime = time(0);
	int count = 0;
	wdet_word* result = wdet_perform_file(h, text_file, &count);
	time_t tEndTime = time(0);
	if(count == 0)
	{
		printf("未提取到结果.\n");
	}

	//显示结果
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

	//释放结果
	wdet_free_result(result);

	//销毁句柄
	wdet_cleanup(h);

	return 0;
}

