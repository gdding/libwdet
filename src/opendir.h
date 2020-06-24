#ifndef _H_OPENDIR_DINGLIN_INCLUDED
#define _H_OPENDIR_DINGLIN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <io.h>
#include <stdlib.h>
#include <sys/types.h>

struct dirent
{   
	long d_ino;   
	off_t d_off;     
	unsigned short d_reclen;   
	char d_name[_MAX_FNAME+1];   
};   

typedef struct
{   
	intptr_t handle;                               
	short offset;                             
	short finished;                         
	struct _finddata_t fileinfo;   
	char *dir;                                       
	struct dirent dent;                     
} DIR;   

DIR* opendir(const char* );   
struct dirent* readdir(DIR* );   
int closedir(DIR* );  

#endif


#ifdef __cplusplus
}
#endif



#endif //_H_OPENDIR_DINGLIN_INCLUDED
