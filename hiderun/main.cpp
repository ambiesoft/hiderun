#include <windows.h>
#include "../../MyUtility/CreateProcessCommon.h"

#define APPNAME L"hiderun"

#pragma intrinsic(memset)
#pragma function(memset)

void* memset(void* dist, int val, size_t size)
{
	BYTE* p = (BYTE*)dist;
	for(size_t i=0 ; i < size ; ++i, ++p)
		*p = val;
	return dist;
}


void entrypoint()
{
	int argc=0;
	LPWSTR* argv=CommandLineToArgvW(GetCommandLine(), &argc);
	if(argc<2)
	{
		LocalFree(argv);
		ExitProcess(1);
	}
	int ret=0;
	if(!CreateProcessCommon(argv[1],NULL, TRUE))//L"C:\\Linkout\\bin\\curr.bat");
	{
		MessageBox(NULL, L"Failed", APPNAME, MB_ICONEXCLAMATION);
		ret=1;
	}
	LocalFree(argv);
	ExitProcess(ret);
}

