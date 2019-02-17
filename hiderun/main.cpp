#include "stdafx.h"

#include <windows.h>

#include "../../lsMisc/CreateProcessCommon.h"
#include "../../lsMisc/Is64.h"


#define APPNAME L"hiderun"

//#pragma intrinsic(memset)
//#pragma function(memset)



//void* memset(void* dist, int val, size_t size)
//{
//	BYTE* p = (BYTE*)dist;
//	for(size_t i=0 ; i < size ; ++i, ++p)
//		*p = val;
//	return dist;
//}

#ifndef _countof
#define _countof(a) sizeof(a)/sizeof(a[0])
#endif

using namespace Ambiesoft;
using namespace std;

LPWSTR myGetLastErrorString(DWORD dwErrorNo)
{
	LPVOID lpMsgBuf = NULL;
 
	if( (0==FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL,
		dwErrorNo,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf,
		0,
		NULL)) || lpMsgBuf==NULL )
	{
		return NULL;
	}
 
	return (LPWSTR)lpMsgBuf;
}

static BOOL iss(TCHAR c)
{
	return c==L' ' || c==L'\t' || c==L'\r' || c==L'\n';
}

static LPTSTR getcommandlargine()
{
	LPTSTR p = GetCommandLine();

	if (p == NULL)
		return NULL;
	if (p[0] == 0)
		return p;

	TCHAR qc=0;
	if(p[0]==L'"' || p[0]==L'\'')
	{
		qc=p[0];
		++p;
	}

	for(; *p ; ++p)
	{
		if(qc)
		{
			if(*p==qc)
			{
				qc=0;
				continue;
			}
			continue;
		}

		if(iss(*p))
		{
			for( ; *p ; ++p)
			{
				if(!iss(*p))
				{
					int count = lstrlen(p);
					LPTSTR pRet = (LPTSTR)LocalAlloc(0, (count+1)*sizeof(TCHAR));
					lstrcpy(pRet,p);
					return pRet;
				}
			}
			return NULL;
		}
	}
	return NULL;
}


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	int argc=0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

	if(argc < 2)
	{
		MessageBox(NULL, L"No Arguments", APPNAME, MB_ICONEXCLAMATION);
		LocalFree(argv);
		return(1);
	}

	if (argc > 1 && (
		lstrcmp(argv[1], L"-h") == 0 ||
		lstrcmp(argv[1], L"/h") == 0 ||
		lstrcmp(argv[1], L"/?") == 0))
	{
		wstring message;
		message += L"hiderun\n";
		message += L"Run console application without showing the console";
		message += L"\n\n";
		message += L"ex)\n";
		message += L"hiderun.exe command";
		MessageBox(NULL, message.c_str(), APPNAME, MB_ICONINFORMATION);
		return 0;
	}
		
	LPCTSTR pArg = getcommandlargine();

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
				if (!CreateProcessCommon(szT, pArg))
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
	DWORD dwLE = 0;
	
	//L"C:\\Linkout\\bin\\curr.bat");
	if(!CreateProcessCommon(pArg,
		NULL,
		TRUE,
		&dwLE,
		WaitProcess_InputIdle,
		10*1000))
	{
		LPWSTR p = (LPWSTR)LocalAlloc(0, 1024);
		p[0]=0;
		lstrcpy(p, L"Failed to launch: ");
		lstrcat(p, argv[1]);
		lstrcat(p, L"\r\n");
		LPCWSTR q = myGetLastErrorString(dwLE);
		lstrcat(p,q);
		MessageBox(NULL, p, APPNAME, MB_ICONEXCLAMATION);
		// MessageBox(NULL, GetCommandLine(), APPNAME, MB_ICONEXCLAMATION);
		// MessageBox(NULL, argv[1], APPNAME, MB_ICONEXCLAMATION);

		LocalFree((void*)q);
		LocalFree(p);
		ret=1;
	}
	// additional wait for a starting process to get focus properly
	Sleep(10*1000);

	LocalFree((void*)pArg);
	LocalFree(argv);
	return (ret);
}

