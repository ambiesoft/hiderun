#include <windows.h>


#include "../../MyUtility/CreateProcessCommon.h"
#include "../../MyUtility/Is64.h"

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


int main()
{
	int argc=0;
	LPWSTR* argv=CommandLineToArgvW(GetCommandLine(), &argc);
	if(argc<2)
	{
		MessageBox(NULL, L"No Arguments", APPNAME, MB_ICONEXCLAMATION);
		LocalFree(argv);
		return(1);
	}

	if (Is64BitWindows() && !Is64BitProcess())
	{
		TCHAR szT[MAX_PATH]; szT[0] = 0;
		GetModuleFileName(NULL, szT, _countof(szT));
		size_t len = lstrlen(szT);
		LPWSTR p = &szT[len] - 1;
		for (; p != szT; --p)
		{
			if (*p == L'\\')
			{
				lstrcpy(p+1, L"hiderun64.exe");
				if (!CreateProcessCommon(szT, argv[1]))
				{
					MessageBox(NULL, L"Failed to launch hiderun64", APPNAME, MB_ICONEXCLAMATION);
					return 1;
				}
				return 0;
			}
		}
		return 1;
	}

	int ret=0;
	if(!CreateProcessCommon(argv[1],NULL, TRUE))//L"C:\\Linkout\\bin\\curr.bat");
	{
		MessageBox(NULL, L"Failed to launch", APPNAME, MB_ICONEXCLAMATION);
		ret=1;
	}
	LocalFree(argv);
	return (ret);
}

void entrypoint()
{
	ExitProcess(main());
}