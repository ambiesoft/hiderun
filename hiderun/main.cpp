#include "stdafx.h"



#include "../../lsMisc/CreateProcessCommon.h"
#include "../../lsMisc/Is64.h"
#include "../../lsMisc/CommandLineString.h"
#include "../../lsMisc/GetLastErrorString.h"




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

//LPWSTR myGetLastErrorString(DWORD dwErrorNo)
//{
//	LPVOID lpMsgBuf = NULL;
// 
//	if( (0==FormatMessageW(
//		FORMAT_MESSAGE_ALLOCATE_BUFFER |
//		FORMAT_MESSAGE_FROM_SYSTEM |
//		FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_MAX_WIDTH_MASK,
//		NULL,
//		dwErrorNo,
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		(LPWSTR)&lpMsgBuf,
//		0,
//		NULL)) || lpMsgBuf==NULL )
//	{
//		return NULL;
//	}
// 
//	return (LPWSTR)lpMsgBuf;
//}

//static BOOL iss(TCHAR c)
//{
//	return c==L' ' || c==L'\t' || c==L'\r' || c==L'\n';
//}
//
//static LPTSTR getcommandlargine()
//{
//	LPTSTR p = GetCommandLine();
//
//	if (p == NULL)
//		return NULL;
//	if (p[0] == 0)
//		return p;
//
//	TCHAR qc=0;
//	if(p[0]==L'"' || p[0]==L'\'')
//	{
//		qc=p[0];
//		++p;
//	}
//
//	for(; *p ; ++p)
//	{
//		if(qc)
//		{
//			if(*p==qc)
//			{
//				qc=0;
//				continue;
//			}
//			continue;
//		}
//
//		if(iss(*p))
//		{
//			for( ; *p ; ++p)
//			{
//				if(!iss(*p))
//				{
//					int count = lstrlen(p);
//					LPTSTR pRet = (LPTSTR)LocalAlloc(0, (count+1)*sizeof(TCHAR));
//					lstrcpy(pRet,p);
//					return pRet;
//				}
//			}
//			return NULL;
//		}
//	}
//	return NULL;
//}

static wstring getHelpString()
{
	wstring message;
	message += L"hiderun\n";
	message += L"Run console application without showing the console";
	message += L"\n\n";
	message += L"ex)\n";
	message += L"hiderun.exe command";

	return message;
}
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	int argc=0;
	std::unique_ptr<LPWSTR, HLOCAL(__stdcall *)(HLOCAL)>
		arg(::CommandLineToArgvW(::GetCommandLine(), &argc), ::LocalFree);
	
	if(argc < 2)
	{
		MessageBox(NULL, L"No Arguments", APPNAME, MB_ICONEXCLAMATION);
		return 1;
	}

	int sleepsec = 0;
	wstring laucharg;
	CCommandLineString cmdline(lpCmdLine);
	for (int i=0; i < cmdline.getCount(); ++i)
	{
		laucharg = cmdline.subString(i);
		wstring line = cmdline.getArg(i);
		if (!line.empty() && line[0] != L'/')
			break;

		if (line == L"/h" || line == L"/?")
		{
			MessageBox(NULL, 
				getHelpString().c_str(), 
				APPNAME_AND_VERSION,
				MB_ICONINFORMATION);
			return 0;
		}
		else if (line == L"/sleep")
		{
			++i;
			sleepsec = stoi(cmdline.getArg(i));
		}
		
	}
	
	
	if (Is64BitWindows() && !Is64BitProcess())
	{
		// open with 64 version
		TCHAR szT[MAX_PATH]; szT[0] = 0;
		GetModuleFileName(NULL, szT, _countof(szT));
		size_t len = lstrlen(szT);
		LPWSTR p = &szT[len] - 1;
		for (; p != szT; --p)
		{
			if (*p == L'\\')
			{
				lstrcpy(p+1, L"hiderun64.exe");
				if (!CreateProcessCommon(szT, cmdline.subString(0).c_str()))
				// if (!CreateProcessCommon(L"C:\\Linkout\\argCheck\\argCheck.exe", pArg))
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
	if(!CreateProcessCommon(laucharg.c_str(),
		NULL,
		TRUE,
		&dwLE,
		WaitProcess_InputIdle,
		10*1000))
	{
		wstring message;
		message += L"Failed to launch: \r\n";
		message += laucharg.c_str();
		message += L"\r\n\r\n";
		message += GetLastErrorString(dwLE);
		
		MessageBox(NULL, message.c_str(), APPNAME, MB_ICONEXCLAMATION);

		return 1;
	}
	
	if (sleepsec > 0)
		Sleep(sleepsec * 1000);

	return (ret);
}

