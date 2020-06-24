#include "Utility.h"
#include "DirScanner.h"


namespace ns_wdet
{

int loadFile(const char *sFilename, char* & pBuffer)
{
    FILE *fp = fopen(sFilename,"rb");
    if (fp == NULL) return -1;
    fseek(fp, 0, SEEK_END);
    int nFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    pBuffer = (char*)calloc(nFileSize+1, sizeof(char));
    fread(pBuffer, 1, nFileSize, fp);
    pBuffer[nFileSize]=0;
    fclose(fp);
    return nFileSize;
}


//获得下一个字符/汉字的内码
int getNextCharCode(const char* sText, int nTextLen, WDETcharset cs, int & nNextCharPos)
{
	if(nNextCharPos >= nTextLen) return -1;

	int iInnerCode = -1;
	if(cs == WDET_CHARSET_GBK)
	{
		int nCharLen = 1;
		unsigned char a = (unsigned char)sText[nNextCharPos];
		if((a & 0x80) == 0x00) //单字节
		{
			nCharLen = 1;
			iInnerCode = a;

			/*
			if (iInnerCode >= 'A' && iInnerCode <= 'Z') // 大写字母转小写字母
			{
				iInnerCode = 'a' + iInnerCode - 'A';
			}
			*/
		}
		else if(nNextCharPos+1 < nTextLen) //双字节
		{
			nCharLen = 2;
			iInnerCode = (a<<8) + (unsigned char)sText[nNextCharPos+1];

			/*
			if (iInnerCode >= 0xA3E1 && iInnerCode <= 0xA3FA) // 小写字符全角
			{
				iInnerCode = 'a' + iInnerCode - 0xA3E1;
			}
			else if (iInnerCode >= 0xA3C1 && iInnerCode <= 0xA3DA) // 大写字符的全角
			{
				iInnerCode = 'a' + iInnerCode - 0xA3C1;
			}
			else if (iInnerCode >= 0xA3B0 && iInnerCode <= 0xA3B9) // 数字字符的全角 
			{
				iInnerCode = '0' + iInnerCode - 0xA3B0;
			}
			*/
		}
		nNextCharPos += nCharLen;
	}
	else if(cs == WDET_CHARSET_UTF8)
	{
		int nCharLen = 1;
		unsigned char a = (unsigned char)sText[nNextCharPos];
		if((a & 0x80) == 0x00) //单字节
		{
			nCharLen = 1;
			iInnerCode = a;

			/*
			if (iInnerCode >= 'A' && iInnerCode <= 'Z') // 大写字母转小写字母
			{
				iInnerCode = 'a' + iInnerCode - 'A';
			}
			*/
		}
		else if((a & 0xE0) == 0xC0) // 双字节 110xxxxx 10xxxxxx
		{
			if(nNextCharPos + 1 < nTextLen)
			{				
				unsigned char b = (unsigned char)sText[nNextCharPos+1];
				if((b & 0xC0)==0x80)
				{
					nCharLen = 2;
					iInnerCode = ((a & 0x1F) << 6) + (b & 0x3F);
				}
			}
		}
		else if((a & 0xF0) == 0xE0) // 三字节 1110xxxx 10xxxxxx 10xxxxxx
		{
			if(2 + nNextCharPos < nTextLen)
			{
				nCharLen = 3;
				unsigned char b = (unsigned char)sText[nNextCharPos+1];
				unsigned char c = (unsigned char)sText[nNextCharPos+2];
				if ((b & 0xC0) == 0x80 && (c & 0xC0) == 0x80)
				{
					nCharLen = 3;
					iInnerCode = ((a & 0x0F) << 12) + ((b & 0x3F) << 6) + (c & 0x3F);

					/*
					if (iInnerCode >= 0xFF41 && iInnerCode <= 0xFF5A) // 小写字符全角
					{
						iInnerCode = 'a' + iInnerCode - 0xFF41;
					}
					else if (iInnerCode >= 0xFF21 && iInnerCode <= 0xFF3A) // 大写字符的全角
					{
						iInnerCode = 'a' + iInnerCode - 0xFF21;
					}
					else if (iInnerCode >= 0xFF10 && iInnerCode <= 0xFF19) // 数字字符的全角 
					{
						iInnerCode = '0' + iInnerCode - 0xFF10;
					}
					*/
				}
			}
		}
		nNextCharPos += nCharLen;
	}

	return iInnerCode;
}

string getCharByCode(int nCharCode, WDETcharset cs)
{
	char sChar[4]="";
	if(cs == WDET_CHARSET_GBK)
	{
		if (nCharCode < 256)
		{
			sChar[0] = (char)nCharCode;
			sChar[1] = 0;
		}
		else
		{
			sChar[0] = nCharCode/256;
			sChar[1] = nCharCode%256;
		}
	}
	else if(cs == WDET_CHARSET_UTF8)
	{
		if(nCharCode < 0x007F)
		{
			sChar[0] = (char)nCharCode;
		}
		else if(nCharCode < 0x07FF)
		{
			sChar[0] = (0xC0 | ((nCharCode>>6)&0x1f));
			sChar[1] = (0x80 | (nCharCode&0x3f));
		}
		else
		{
			sChar[0] = (0xE0 | ((nCharCode>>12)&0x0f));
			sChar[1] = (0x80 | ((nCharCode>>6)&0x3f));
			sChar[2] = (0x80 | (nCharCode&0x3f));
		}
	}
	return string(sChar);
}


string GenRandomString(int n)
{
    static unsigned int seed = 1;
    srand((unsigned int)time(0) + (seed++));
    string s;
    const int RANDOM_SET_SIZE = 36;
	static char sChars[]="0123456789abcdefghijklmnopqrstuvwxyz";
    for(int i=0; i < n; i++)
    {
	    s += sChars[rand()%RANDOM_SET_SIZE];
    }
    return s;
}

bool isBigEndian()
{
	unsigned short s = 0x1234;
	return (htons(s) == s);
}

unsigned long long htonll(unsigned long long x)
{
	if(isBigEndian())
		return x;

	unsigned long long y;
	unsigned int a = x >> 32;
	unsigned int b = x & 0xFFFFFFFFLL;

	a = htonl(a);
	b = htonl(b);
	y = b;
	y <<= 32;
	y |= a;
	return y;
}

bool encrypt_buffer(char* pData, unsigned int nDataLen, const char* pKey, unsigned int nKeyLen)
{
	const unsigned long long KEYKEY = htonll(8274273513691587185LL);
	if(pKey == NULL || nKeyLen == 0)
		return false;

	char* pKeyKey = (char*)malloc(nKeyLen);
	if(pKeyKey == NULL) return false;
	memcpy(pKeyKey, pKey, nKeyLen);

	unsigned int i=0,j=0;
	while(i < nKeyLen)
	{
		pKeyKey[i] = pKeyKey[i] ^ ((unsigned char*)&KEYKEY)[j];
		i++; j++;
		if(j == sizeof(KEYKEY))
		{
			j = 0;
		}
	}

	i=0; j=0;
	while(i < nDataLen)
	{
		pData[i] = pData[i]^pKeyKey[j]^*((char*)&i);
		i++; j++;
		if(j == nKeyLen)
		{
			j = 0;
		}
	}

	free(pKeyKey);
	return true;
}

static const char* ENCRYPT_FILE_HEADER = ".encrypt.dgd";
static const char* ENCRYPT_PASSWORD = "0123456789abcdefghijklmnopqrstuvwxyz";
static const int ENCRYPT_PASSWORD_SIZE = 36;
static const int RANDOM_KEY_SIZE = 32;
bool encrypt_file(const char* from_file, const char* to_file)
{
	//读取源文件
	char* pcDataBuf = NULL;
	int nDataSize = loadFile(from_file, pcDataBuf);
	if(nDataSize < 0) return false;

	if(!CreateFilePath(to_file))
	{
		printf("CreateFilePath failed!\n");
		return false;
	}
	FILE* fpToFile = fopen(to_file, "wb");
	if(fpToFile == NULL)
	{
		printf("Can't create the file: %s\n", to_file);
		free(pcDataBuf);
		return false;
	}

	//若已为加密文件则不再需要进行加密处理
	size_t nWriteSize = 0;
	if(strstr(pcDataBuf, ENCRYPT_FILE_HEADER) != pcDataBuf)
	{
		//先写头部
		nWriteSize = fwrite(ENCRYPT_FILE_HEADER, 1, strlen(ENCRYPT_FILE_HEADER), fpToFile);
		assert(nWriteSize == strlen(ENCRYPT_FILE_HEADER));

		//生成随机密钥并对其加密后再写入文件
		string sRandomKey = GenRandomString(RANDOM_KEY_SIZE);
		char* pcRandomKey = strdup(sRandomKey.c_str());
		encrypt_buffer(pcRandomKey, RANDOM_KEY_SIZE, ENCRYPT_PASSWORD, ENCRYPT_PASSWORD_SIZE);
		nWriteSize = fwrite(pcRandomKey, 1, RANDOM_KEY_SIZE, fpToFile);
		assert(nWriteSize == RANDOM_KEY_SIZE);
		free(pcRandomKey);

		//根据随机密钥加密原数据
		encrypt_buffer(pcDataBuf, nDataSize, sRandomKey.c_str(), RANDOM_KEY_SIZE);

		//输出加密结果到目标文件
		nWriteSize = fwrite(pcDataBuf, 1, nDataSize, fpToFile);
		assert(nWriteSize == nDataSize);
	}
	else
	{
		//源文件已加密的直接输出
		nWriteSize = fwrite(pcDataBuf, 1, nDataSize, fpToFile);
		assert(nWriteSize == nDataSize);
	}
	
	free(pcDataBuf);
	fclose(fpToFile);
	return true;
}

bool encrypt_dir(const char* from_dir, const char* to_dir)
{
	assert(from_dir != NULL);
	string sFromDir = from_dir;
	NormalizePath(sFromDir, true);

	assert(to_dir != NULL);
	string sToDir = to_dir;
	NormalizePath(sToDir, true);

	CDirScanner ds(sFromDir.c_str());
	const vector<string>& files = ds.GetFileList();
	for(size_t i = 0; i < files.size(); i++)
	{
		const string& sFromFile = files[i];
		string sFromFilePath = sFromDir + sFromFile;
		string sToFilePath = sToDir + sFromFile;		
		if(!encrypt_file(sFromFilePath.c_str(), sToFilePath.c_str())) return false;
	}

	return true;
}

int loadEncryptFile(const char* file, char* & pBuffer)
{
	//读取源文件
	char* pcFileBuf = NULL;
	int nFileSize = loadFile(file, pcFileBuf);
	if(nFileSize < 0) return -1;
	if(strstr(pcFileBuf, ENCRYPT_FILE_HEADER) != pcFileBuf)
	{
		//若头部不对则直接读取文件返回
		pBuffer = pcFileBuf;
		return nFileSize;
	}

	//得到随机密钥
	char* pcRandomKey = pcFileBuf + strlen(ENCRYPT_FILE_HEADER);
	encrypt_buffer(pcRandomKey, RANDOM_KEY_SIZE, ENCRYPT_PASSWORD, ENCRYPT_PASSWORD_SIZE);

	//使用随机密钥解密数据
	char* pcData = pcRandomKey + RANDOM_KEY_SIZE;
	int nDataSize = nFileSize - (int)strlen(ENCRYPT_FILE_HEADER) - RANDOM_KEY_SIZE;
	assert(nDataSize >= 0);
	encrypt_buffer(pcData, nDataSize, pcRandomKey, RANDOM_KEY_SIZE);

	//输出结果
	pBuffer = (char*)malloc(nDataSize+1);
	assert(pBuffer != NULL);
	memset(pBuffer, 0, nDataSize+1);
	memcpy(pBuffer, pcData, nDataSize);

	free(pcFileBuf);
	return nDataSize;
}



void NormalizePath(char* dir, bool bTail)
{
	size_t len = strlen(dir);
#ifdef _WIN32
	for(size_t m=0; m<len; m++)
	{
		if(dir[m]=='/') dir[m]='\\';
	}
	if(bTail)
	{
		if(dir[len-1] != '\\')
		{
			dir[len] = '\\';
			dir[len+1] = 0;
		}
	}
#else
	for(size_t m=0; m<len; m++)
	{
		if(dir[m]=='\\') dir[m]='/';
	}
	if(bTail)
	{
		if(dir[len-1] != '/')
		{
			dir[len] = '/';
			dir[len+1] = 0;
		}
	}
#endif
}

void NormalizePath(string& dir, bool bTail)
{
	size_t len = dir.length();
#ifdef _WIN32
	for(size_t m=0; m<len; m++)
	{
		if(dir[m]=='/') dir[m]='\\';
	}
	if(bTail)
	{
		if(dir[len-1] != '\\')
		{
			dir += '\\';
		}
	}
#else
	for(size_t m=0; m<len; m++)
	{
		if(dir[m]=='\\') dir[m]='/';
	}
	if(bTail)
	{
		if(dir[len-1] != '/')
		{
			dir += '/';
		}
	}
#endif
}

bool CreateFilePath(const char* path)
{
	if(path==NULL || path[0]==0)  return false;

	string sCurPath("");
	int i = 0;
	char ch = path[i];
	while(ch != 0)
	{
		sCurPath += ch;
		if(ch == '/' || ch == '\\')
		{
			if(!DIR_EXIST(sCurPath.c_str()) && (MAKE_DIR(sCurPath.c_str()) != 0))
			{
				return false;
			}
		}
		ch = path[++i];
	}
	return true;
}

void parseTextByLine(const char* text, int size, vector<string>& result)
{
	int i = 0;
	while(i < size)
	{
		while(text[i] == 0x0A && i < size) i++;
		while(text[i] == 0x0D && i < size) i++;

		string s = "";
		int j = i;
		for(; j < size; j++)
		{
			const char& c = text[j];
			if(c == 0x0D || c == 0x0A) break;
			s += c;
		}
		if(!s.empty()) result.push_back(s);
		if(j >= size) break;

		i = j;
	}
}

bool isLetter(char ch)
{
	return (ch<='Z' && ch>='A') || (ch<='z' && ch>='a');
}

void toLower(string & s)
{
	for(int i=0; i<(int)s.length(); i++)
	{
		if(isLetter(s[i]))
		{
			s[i] = (char)tolower(s[i]);
		}
	}
}


}
