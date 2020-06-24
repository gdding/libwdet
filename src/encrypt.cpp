#include "utils/Utility.h"
#include <stdio.h>

#pragma comment(lib, "utils")

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		printf("Usage:  %s <flag> <from> <to>\n", argv[0]);
		printf("Where: \n    flag - 0 for file, 1 for directory\n");
		printf("    from - path of file or directory to be encrypted\n");
		printf("    to - path of file or directory to save the result\n");
		return 0;
	}

	int flag = atoi(argv[1]);
	if(flag == 0)
		ns_wdet::encrypt_file(argv[2], argv[3]);
	else if(flag == 1)
		ns_wdet::encrypt_dir(argv[2], argv[3]);

	printf("OK\n");

	return 0;
}

