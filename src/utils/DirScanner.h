/*-----------------------------------------------------------------------------
* Copyright (c) 2010 ICT, CAS. All rights reserved.
*   dingguodong@software.ict.ac.cn
*
* This file is the confidential and proprietary property of 
* ICT, CAS and the posession or use of this file requires 
* a written license from the author.

* Last-Modified: 2010-03-04
*-----------------------------------------------------------------------------*/
#ifndef _H_DIRSCANNER_GDDING_INCLUDED_20100304
#define _H_DIRSCANNER_GDDING_INCLUDED_20100304
#include "StdHeader.h"

namespace ns_wdet
{

class CDirScanner
{
public:
	CDirScanner(const char* dir, bool bRecursive=true);

public:
	//��ñ�ɨ���Ŀ¼
	const string& GetBaseDir(){return m_sBaseDir;}

	//��ø�Ŀ¼��ɨ�赽���ļ����·���б�������Ŀ¼����
	const vector<string>& GetFileList(){return m_vFileList;}

	//��ø�Ŀ¼��ɨ�赽���ļ������·���б�������Ŀ¼����
	const vector<string>& GetDirList(){return m_vDirList;}

	//��ø�Ŀ¼��ɨ�赽���������·���б�������Ŀ¼����
	const vector<string>& GetAllList(){return m_vAllList;}

	//��ø�Ŀ¼��ɨ�赽���ļ�����·���б�������Ŀ¼����
	const vector<string>& GetFileFullList(){return m_vFileFullList;}

protected:
	void perform(const char *dir, bool bRecursive=true);
	virtual void OnFileFound(const char *filename);
	virtual void OnSubDirFound(const char *currentdir, const char *parentdir);

private:
	vector<string> m_vFileList;	//�ļ����·���б�
	vector<string> m_vDirList;	//�ļ������·���б�
	vector<string> m_vAllList;	//���е��б����ļ����ļ��У�
	vector<string> m_vFileFullList;//�ļ�����·���б�

private:
	string m_sBaseDir;
	int m_bRecursive;
};

}

#endif //_H_DIRSCANNER_GDDING_INCLUDED_20100304
