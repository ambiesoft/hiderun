#include "stdafx.h"

#include "../../lsMisc/CreateProcessCommon.h"
#include "../../lsMisc/Is64.h"
#include "../../lsMisc/CommandLineString.h"
#include "../../lsMisc/GetLastErrorString.h"

#ifndef _countof
#define _countof(a) sizeof(a)/sizeof(a[0])
#endif

using namespace Ambiesoft;
using namespace std;

static wstring getHelpString()
{
	wstring message;
	message += L"hiderun\n";
	message += I18N(L"Run console application without showing the console");
	message += L"\n\n";
	message += L"ex)\n";
	message += L"hiderun.exe command";

	return message;
}

bool ReadFileBytes(HANDLE hFile, LPVOID lpBuffer, DWORD dwSize)
{
	DWORD dwBytes = 0;

	if (!ReadFile(hFile, lpBuffer, dwSize, &dwBytes, NULL)) {
		// TRACE(_T("Failed to read file!\n"));
		return (false);
	}

	if (dwSize != dwBytes) {
		// TRACE(_T("Wrong number of bytes read, expected\n"), dwSize, dwBytes);
		return (false);
	}

	return (true);
}
int GetSubsystemFromImage(LPCWSTR m_stFilePath)
{
	HANDLE hImage;
	DWORD dwCoffHeaderOffset;
	DWORD dwNewOffset;
	DWORD dwMoreDosHeader[16];
	ULONG ulNTSignature;

	IMAGE_DOS_HEADER dos_header;
	IMAGE_FILE_HEADER file_header;
	IMAGE_OPTIONAL_HEADER optional_header;

	// Open the application file.

	hImage = CreateFile(m_stFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hImage == INVALID_HANDLE_VALUE) {
		// AfxMessageBox(_T("Failed to open the aplication file!\n"));
		return -1;
	}

	// Read MS-Dos image header.

	if (!ReadFileBytes(hImage, &dos_header, sizeof(IMAGE_DOS_HEADER))) {
		// AfxMessageBox(_T("Failed to read file!\n"));
		return -1;
	}

	if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
		// AfxMessageBox(_T("Application failed to classify the file type!\n"));
		return -1;
	}

	// Read more MS-Dos header.

	if (!ReadFileBytes(hImage, dwMoreDosHeader, sizeof(dwMoreDosHeader))) {
		// AfxMessageBox(_T("Failed to read file!\n"));
		return -1;
	}

	// Move the file pointer to get the actual COFF header.

	dwNewOffset = SetFilePointer(hImage, dos_header.e_lfanew, NULL, FILE_BEGIN);
	dwCoffHeaderOffset = dwNewOffset + sizeof(ULONG);

	if (dwCoffHeaderOffset == 0xFFFFFFFF) {
		// AfxMessageBox(_T("Failed to move file pointer!\n"));
		return -1;
	}

	// Read NT signature of the file.
	if (!ReadFileBytes(hImage, &ulNTSignature, sizeof(ULONG))) {
		// AfxMessageBox(_T("Failed to read NT signature of file!\n"));
		return -1;
	}

	if (ulNTSignature != IMAGE_NT_SIGNATURE) {
		// AfxMessageBox(_T("Missing NT signature!\n"));
		return -1;
	}

	if (!ReadFileBytes(hImage, &file_header, IMAGE_SIZEOF_FILE_HEADER)) {
		// AfxMessageBox(_T("Failed to read file!\n"));
		return -1;
	}

	// Read the optional header of file.
#define IMAGE_SIZEOF_NT_OPTIONAL32_HEADER    224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER    240

#ifdef _WIN64
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER IMAGE_SIZEOF_NT_OPTIONAL64_HEADER
#else
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER IMAGE_SIZEOF_NT_OPTIONAL32_HEADER
#endif

	if (!ReadFileBytes(hImage, &optional_header, IMAGE_SIZEOF_NT_OPTIONAL_HEADER)) {
		// AfxMessageBox(_T("Failed to read file for optional header!\n"));
		return -1;
	}

	return optional_header.Subsystem;
	//case IMAGE_SUBSYSTEM_UNKNOWN:
	//	m_stAppType = _T("Unknown");
	//	break;
	//case IMAGE_SUBSYSTEM_NATIVE:
	//	m_stAppType = _T("Native");
	//	break;
	//case IMAGE_SUBSYSTEM_WINDOWS_GUI:
	//	m_stAppType = _T("Windows GUI");
	//	break;
	//case IMAGE_SUBSYSTEM_WINDOWS_CUI:
	//	m_stAppType = _T("Windows Console");
	//	break;
	//case IMAGE_SUBSYSTEM_OS2_CUI:
	//	m_stAppType = _T("OS//2 Console");
	//	break;
	//case IMAGE_SUBSYSTEM_POSIX_CUI:
	//	m_stAppType = _T("Posix Console");
	//	break;
	//case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:
	//	m_stAppType = _T("Native Win9x");
	//	break;
	//case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
	//	m_stAppType = _T("Windows CE GUI");
	//	break;
	//}
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
		MessageBox(NULL, 
			(I18N(L"No Arguments") + (L"\r\n\r\n" + getHelpString())).c_str(),
			APPNAME,
			MB_ICONEXCLAMATION);
		return 1;
	}

	int sleepsec = 0;
	wstring laucharg;
	CCommandLineString cmdline(lpCmdLine);
	for (size_t i=0; i < cmdline.getCount(); ++i)
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
	
	// check subsystem
	// TODO: check
	GetSubsystemFromImage(laucharg.c_str());

	int ret=0;
	DWORD dwLE = 0;
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

