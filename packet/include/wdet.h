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
	WDET_OK					= 0,	/* �޴��� */
	WDET_BAD_FUNC_PARAM		= 9000,	/* �����������Ϸ� */
	WDET_INVALID_OPTION		= 9001,	/* �Ƿ���ѡ�� */
	WDET_FILE_READ_FAILED	= 9002,	/* �ļ���ȡ���� */
	WDET_OUT_OF_MEMORY		= 9003	/* �ڴ�������� */
}WDETcode;

typedef enum
{
	WDET_CHARSET_GBK = 0,
	WDET_CHARSET_UTF8 = 1
}WDETcharset;

typedef enum
{
	/* ���ô������ı��ı����ַ�����ȱʡΪWDET_CHARSET_GBK
	 * �磺wdet_setopt(h, WDETOPT_CHARSET, WDET_CHARSET_UTF8); */
	WDETOPT_CHARSET = 1000,

	/* �������ʳ������ٸ��֣���ȱʡΪ10������ 
	 * �磺wdet_setopt(h,WDETOPT_MAX_WORD_LENGTH,8); */
	WDETOPT_MAX_WORD_LENGTH = 1001,

	/* ����AVֵ��С��ֵ��AVֵС�ڸ�ֵ�Ľ��������ȱʡΪ3 
	 * �磺wdet_setopt(h, WDETOPT_MIN_AVTHRESH, 4); */
	WDETOPT_MIN_AVTHRESH = 1002,

	/* ���ûص��������ж�һ�����Ƿ�Ϊ���дʣ�
	 * �����øûص�����ʱ����������з��ֵĴ�
	 * ��������Ϊcb_existed_word */
	WDETOPT_WORD_EXISTED_FUNC = 1003,

	/* ����ͣ���ֹ����ļ�����һ�����к������е�һ�����򲻻�����ô���
	 * ���ļ��������ͱ�����WDETOPT_CHARSET���õ���ͬ��
	 * ȱʡΪ"./filter_gbk/stopchar.txt"���û��������Լ���ͣ���ֹ����ļ���
	 * �磺wdet_setopt(h, WDETOPT_STOPCHAR_FILTER_FILE, "./myfilter/stopchar.txt"); */
	WDETOPT_STOPCHAR_FILTER_FILE = 1004,

	/* ����ͣ�ôʹ����ļ�����һ�����к������е�һ�����򲻻�����ô���
	 * ���ļ��������ͱ�����WDETOPT_CHARSET���õ���ͬ��
	 * ȱʡΪ"./filter_gbk/stopword.txt"���û��������Լ���ͣ�ôʹ����ļ���
	 * �磺wdet_setopt(h, WDETOPT_STOPWORD_FILTER_FILE, "./myfilter/stopword.txt"); */
	WDETOPT_STOPWORD_FILTER_FILE = 1005,

	/* �������ֹ����ļ�����һ����������Ϊ���е�һ�����򲻻�����ô��� 
	 * ���ļ��������ͱ�����WDETOPT_CHARSET���õ���ͬ��
	 * ȱʡΪ"./filter_gbk/firstchar.txt"���û��������Լ������ֹ����ļ���
	 * �磺wdet_setopt(h, WDETOPT_FIRSTCHAR_FILTER_FILE, "./myfilter/firstchar.txt"); */
	WDETOPT_FIRSTCHAR_FILTER_FILE = 1006,

	/* ����β�ֹ����ļ�����һ������β��Ϊ���е�һ�����򲻻�����ô���
	 * ���ļ��������ͱ�����WDETOPT_CHARSET���õ���ͬ��
	 * ȱʡΪ"./filter_gbk/finalchar.txt"���û��������Լ���β�ֹ����ļ���
	 * �磺wdet_setopt(h, WDETOPT_FINALCHAR_FILTER_FILE, "./myfilter/finalchar.txt"); */
	WDETOPT_FINALCHAR_FILTER_FILE = 1007,

	/* ������β�ָ��ʹ����ļ���һ����λ�ڴ��׼���β�ĸ��ʣ�
	 * ���ļ��������ͱ�����WDETOPT_CHARSET���õ���ͬ��
	 * ȱʡ����²���ʹ�øù��ܡ�
	 * �磺wdet_setopt(h, WDETOPT_CHARPROB_FILTER_FILE, "./myfilter/charprob.txt"); */
	WDETOPT_CHARPROB_FILTER_FILE = 1008,

}WDETOption;

typedef struct
{
	char	word[WDET_MAX_WORD_SIZE]; //�ʴ�
	double	score;	//����ֵ
	int		freq;		//����Ƶ��
	int		leftcount;	//�����ֵ���Ŀ
	int		rightcount;	//�����ֵ���Ŀ
}wdet_word;


/**
 * ���ܣ���ʼ�������WDET���
 * ������cs - �������ı��ı������ͣ�ֻ֧��UTF8��GBK��
 * ���أ����ɹ��򷵻��ڲ�������WDET��������򷵻�NULL
 * ��ע������������������ֻ����һ��WDET���Ȼ�󵽴�ʹ�ã����̰߳�ȫ��
 *       ��������Ҫʹ��WDETʱ�ǵõ���wdet_cleanup�ͷ���Դ
 **/
WDET_API WDET* wdet_init();

/**
 * ���ܣ�ΪWDET�������ѡ�����
 * ������h - ͨ��wdet_init�����ľ��
 *        opt - �����õ�ѡ�����
 *        ... - ����opt������Ӧ�Ĳ���ֵ
 * ���أ����ɹ��򷵻�WDET_OK�����򷵻ش�����
 * ��ע������WDETOption�����еĸ���ѡ���
 **/
WDET_API WDETcode wdet_setopt(WDET* h, WDETOption opt, ...);

/** 
 * ���ܣ��Ӹ����������ı���ȡ���´�/�����崮
 * ������h - ͨ��wdet_init�����ľ��
 *        text - ��������ı�
 *        size - text�Ĵ�С���ֽ�����
 *        count - ����int������ַ, �洢�������
 * ���أ�������飬��δ��ȡ���������ִ����򷵻�NULL����ʱcountΪ0
 * ��ע��ִ����ɺ�ǵõ���wdet_free_result�ͷŽ����ռ���ڴ�ռ�
 * ������
 *       WDET* h = wdet_init();
 *       int count = 0;
 *       wdet_word* result = wdet_perform(h, "�����ı�", 8, &count);
 *       for(int i=0; i < count; i++)
 *       {
 *            printf("%s\n", result[i].word);
 *       }
 *       wdet_free_result(result);
 *       wdet_cleanup(h);
 **/
WDET_API wdet_word* wdet_perform(WDET* h, const char* text, int size, int* count);

/**
 * ��һ���ı��ļ�����ȡ���´�/�����崮
 * ʹ��˵���ο�wdet_perform��ע��
 **/
WDET_API wdet_word* wdet_perform_file(WDET* h, const char* txtfile, int* count);

/**
 * ���ܣ��ͷ�result��ռ���ڴ�
 * ������result - ��wdet_perform��wdet_end_stream�����صĴ�����
 **/
WDET_API void wdet_free_result(wdet_word* result);

/** 
 * ���ܣ�������wdet_init��������WDET��� 
 **/
WDET_API void wdet_cleanup(WDET* h);


/******************************************************************************
 * ����Ϊ�ı�������ʽ�µļ����ӿں�����
 * ͨ�����ַ�ʽ����Դӹ�ģ���������ǧ����ı�����ȡ���´�/�����崮          
 ******************************************************************************/

/**
 * ���ܣ���ʼ�ı���������þ��
 * ������h - ͨ��wdet_init������WDET���
 * ���أ����ɹ��򷵻�WDETM��������򷵻�NULL
 **/
WDET_API WDETM* wdet_stream_start(WDET* h);

/**
 * ���ܣ����һƪ�������ı�
 * ������h - ͨ��wdet_init������WDET���
 *        hm - ͨ��wdet_stream_start��õ�WDETM���
 *        text - �������ı�
 *        size - �������ı���С���ֽ�����
 * ���أ��ɹ��򷵻�WDET_OK�����򷵻ش�����
 **/
WDET_API WDETcode wdet_stream_perform(WDET* h, WDETM* hm, const char* text, int size);

/**
 * ���һƪ��������ı��ļ�
 **/
WDET_API WDETcode wdet_stream_perform_file(WDET* h, WDETM* hm, const char* txtfile);

/**
 * ���ܣ������ı��������õ�������
 * ������h - ͨ��wdet_init������WDET���
 *        hm - ͨ��wdet_stream_start��õ�WDETM���
 *        count - ����������
 * ��ע��ִ����ɺ�ǵõ���wdet_free_result�ͷ�result��ռ���ڴ�ռ�
 **/
WDET_API wdet_word* wdet_stream_end(WDET* h, WDETM* hm, int* count);


#endif //_H_WDET_GDDING_INCLUDED_20071217
