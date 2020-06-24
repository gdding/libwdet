/*---------------------------------------------------------------
* Copyright (c) 2007~2010 Ding Guodong. All rights reserved.
*   dingguodong@software.ict.ac.cn; gdding@hotmail.com
*
* This file is the confidential and proprietary property of 
* Ding Guodong and the posession or use of this file requires 
* a written license from the author.

* Last-Modified: 2010-12-04
*---------------------------------------------------------------*/
#ifndef _H_WDET_GDDING_INCLUDED_20071217
#define _H_WDET_GDDING_INCLUDED_20071217

#ifdef WIN32
	#ifndef WDET_API
		#define WDET_API extern "C" __declspec(dllimport)
	#endif
#else
	#ifndef WDET_API
		#define WDET_API
	#endif
#endif


#define WDET_MAX_CHARSET_SIZE	65536
#define WDET_MAX_WORD_SIZE		64


#ifndef max
	#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
	#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef NULL
	#define NULL 0
#endif

typedef void WDET;
typedef void WDETM;
typedef bool (*cb_existed_word)(const char* word);

typedef enum
{
	WDET_OK					= 0,	/* 无错误 */
	WDET_BAD_FUNC_PARAM		= 9000,	/* 函数参数不合法 */
	WDET_INVALID_OPTION		= 9001,	/* 非法的选项 */
	WDET_FILE_READ_FAILED	= 9002,	/* 文件读取出错 */
	WDET_OUT_OF_MEMORY		= 9003	/* 内存申请出错 */
}WDETcode;

typedef enum
{
	WDET_CHARSET_GBK = 0,
	WDET_CHARSET_UTF8 = 1
}WDETcharset;

typedef enum
{
	/* 设置待处理文本的编码字符集，缺省为WDET_CHARSET_GBK
	 * 如：wdet_setopt(h, WDETOPT_CHARSET, WDET_CHARSET_UTF8); */
	WDETOPT_CHARSET = 1000,

	/* 设置最大词长（多少个字），缺省为10个汉字 
	 * 如：wdet_setopt(h,WDETOPT_MAX_WORD_LENGTH,8); */
	WDETOPT_MAX_WORD_LENGTH = 1001,

	/* 设置AV值最小阈值，AV值小于该值的将不输出，缺省为3 
	 * 如：wdet_setopt(h, WDETOPT_MIN_AVTHRESH, 4); */
	WDETOPT_MIN_AVTHRESH = 1002,

	/* 设置回调函数（判断一个词是否为已有词）
	 * 不设置该回调函数时，将输出所有发现的词
	 * 函数类型为cb_existed_word */
	WDETOPT_WORD_EXISTED_FUNC = 1003,

	/* 设置停用字过滤文件（若一个串中含有其中的一个字则不会输出该串）
	 * 该文件编码类型必须与WDETOPT_CHARSET设置的相同。
	 * 缺省为"./filter_gbk/stopchar.txt"，用户可设置自己的停用字过滤文件。
	 * 如：wdet_setopt(h, WDETOPT_STOPCHAR_FILTER_FILE, "./myfilter/stopchar.txt"); */
	WDETOPT_STOPCHAR_FILTER_FILE = 1004,

	/* 设置停用词过滤文件（若一个串中含有其中的一个词则不会输出该串）
	 * 该文件编码类型必须与WDETOPT_CHARSET设置的相同。
	 * 缺省为"./filter_gbk/stopword.txt"，用户可设置自己的停用词过滤文件。
	 * 如：wdet_setopt(h, WDETOPT_STOPWORD_FILTER_FILE, "./myfilter/stopword.txt"); */
	WDETOPT_STOPWORD_FILTER_FILE = 1005,

	/* 设置首字过滤文件（若一个串的首字为其中的一个字则不会输出该串） 
	 * 该文件编码类型必须与WDETOPT_CHARSET设置的相同。
	 * 缺省为"./filter_gbk/firstchar.txt"，用户可设置自己的首字过滤文件。
	 * 如：wdet_setopt(h, WDETOPT_FIRSTCHAR_FILTER_FILE, "./myfilter/firstchar.txt"); */
	WDETOPT_FIRSTCHAR_FILTER_FILE = 1006,

	/* 设置尾字过滤文件（若一个串的尾字为其中的一个字则不会输出该串）
	 * 该文件编码类型必须与WDETOPT_CHARSET设置的相同。
	 * 缺省为"./filter_gbk/finalchar.txt"，用户可设置自己的尾字过滤文件。
	 * 如：wdet_setopt(h, WDETOPT_FINALCHAR_FILTER_FILE, "./myfilter/finalchar.txt"); */
	WDETOPT_FINALCHAR_FILTER_FILE = 1007,

	/* 设置首尾字概率过滤文件（一个字位于词首及词尾的概率）
	 * 该文件编码类型必须与WDETOPT_CHARSET设置的相同。
	 * 缺省情况下不会使用该功能。
	 * 如：wdet_setopt(h, WDETOPT_CHARPROB_FILTER_FILE, "./myfilter/charprob.txt"); */
	WDETOPT_CHARPROB_FILTER_FILE = 1008,

}WDETOption;

typedef struct
{
	char	word[WDET_MAX_WORD_SIZE]; //词串
	double	score;	//评分值
	int		freq;		//出现频度
	int		leftcount;	//左临字的数目
	int		rightcount;	//右临字的数目
}wdet_word;


/**
 * 功能：初始化，获得WDET句柄
 * 参数：cs - 待处理文本的编码类型（只支持UTF8或GBK）
 * 返回：若成功则返回内部创建的WDET句柄，否则返回NULL
 * 备注：可以在整个进程中只创建一个WDET句柄然后到处使用（多线程安全）
 *       当不再需要使用WDET时记得调用wdet_cleanup释放资源
 **/
WDET_API WDET* wdet_init();

/**
 * 功能：为WDET句柄设置选项参数
 * 参数：h - 通过wdet_init创建的句柄
 *        opt - 待设置的选项参数
 *        ... - 根据opt填入相应的参数值
 * 返回：若成功则返回WDET_OK，否则返回错误码
 * 备注：参阅WDETOption定义中的各种选项含义
 **/
WDET_API WDETcode wdet_setopt(WDET* h, WDETOption opt, ...);

/** 
 * 功能：从给定的中文文本提取出新词/有意义串
 * 参数：h - 通过wdet_init创建的句柄
 *        text - 待处理的文本
 *        size - text的大小（字节数）
 *        count - 传入int变量地址, 存储结果个数
 * 返回：结果数组，若未抽取到结果或出现错误则返回NULL，此时count为0
 * 备注：执行完成后记得调用wdet_free_result释放结果所占的内存空间
 * 举例：
 *       WDET* h = wdet_init();
 *       int count = 0;
 *       wdet_word* result = wdet_perform(h, "测试文本", 8, &count);
 *       for(int i=0; i < count; i++)
 *       {
 *            printf("%s\n", result[i].word);
 *       }
 *       wdet_free_result(result);
 *       wdet_cleanup(h);
 **/
WDET_API wdet_word* wdet_perform(WDET* h, const char* text, int size, int* count);

/**
 * 从一个文本文件中提取出新词/有意义串
 * 使用说明参考wdet_perform的注释
 **/
WDET_API wdet_word* wdet_perform_file(WDET* h, const char* txtfile, int* count);

/**
 * 功能：释放result所占的内存
 * 参数：result - 由wdet_perform或wdet_end_stream所返回的处理结果
 **/
WDET_API void wdet_free_result(wdet_word* result);

/** 
 * 功能：销毁由wdet_init所创建的WDET句柄 
 **/
WDET_API void wdet_cleanup(WDET* h);


/******************************************************************************
 * 以下为文本流处理方式下的几个接口函数，
 * 通过这种方式你可以从规模达数万或数千万的文本中提取出新词/有意义串          
 ******************************************************************************/

/**
 * 功能：开始文本流处理，获得句柄
 * 参数：h - 通过wdet_init创建的WDET句柄
 * 返回：若成功则返回WDETM句柄，否则返回NULL
 **/
WDET_API WDETM* wdet_stream_start(WDET* h);

/**
 * 功能：添加一篇待处理文本
 * 参数：h - 通过wdet_init创建的WDET句柄
 *        hm - 通过wdet_stream_start获得的WDETM句柄
 *        text - 待处理文本
 *        size - 待处理文本大小（字节数）
 * 返回：成功则返回WDET_OK，否则返回错误码
 **/
WDET_API WDETcode wdet_stream_perform(WDET* h, WDETM* hm, const char* text, int size);

/**
 * 添加一篇待处理的文本文件
 **/
WDET_API WDETcode wdet_stream_perform_file(WDET* h, WDETM* hm, const char* txtfile);

/**
 * 功能：结束文本流处理，得到处理结果
 * 参数：h - 通过wdet_init创建的WDET句柄
 *        hm - 通过wdet_stream_start获得的WDETM句柄
 *        count - 处理结果个数
 * 备注：执行完成后记得调用wdet_free_result释放result所占的内存空间
 **/
WDET_API wdet_word* wdet_stream_end(WDET* h, WDETM* hm, int* count);


#endif //_H_WDET_GDDING_INCLUDED_20071217
