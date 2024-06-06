#include "stdafx.h"

#include "../../lsMisc/CreateProcessCommon.h"
#include "../../lsMisc/Is64.h"
#include "../../lsMisc/CommandLineString.h"
#include "../../lsMisc/GetLastErrorString.h"
#include "../../lsMisc/stdosd/stdosd.h"
#include "../../lsMisc/CHandle.h"
#include "../../lsMisc/GetVersionStringFromResource.h"

#ifndef _countof
#define _countof(a) sizeof(a)/sizeof(a[0])
#endif

#pragma comment(lib, "Shlwapi.lib")

using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;
using namespace std;

static wstring getHelpString()
{
	wstring message;
	message += L"hiderun\n";
	message += I18N(L"Run a console application without showing the console");
	message += L"\n\n";
	message += L"ex)\n";
	message += L"hiderun.exe [/h|/?] [/v] [/w] command [args...]\n\n";
	message += L"/w Wait for the invoked process to finish\n";
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

//wstring GetLaunchingPath(LPCWSTR pPath)
//{
//	if (stdFileExists(pPath))
//		return pPath;
//
//	wchar_t path[MAX_PATH];
//	vector<wstring> tries;
//	wstring ext = stdGetFileExtension(pPath);
//	if(ext==L".com" || ext==L".exe")
//	{
//		tries.push_back(pPath);
//	}
//	else
//	{
//		tries.push_back(pPath);
//		tries.push_back(pPath + wstring(L".com"));
//		tries.push_back(pPath + wstring(L".exe"));
//	}
//
//	CHModule hm(NULL);
//	for (auto&& path : tries)
//	{
//		HMODULE h = LoadLibraryEx(path.c_str(), NULL,
//			DONT_RESOLVE_DLL_REFERENCES // LOAD_LIBRARY_AS_DATAFILE makes GetModuleFileName fail
//			);
//		if (h)
//		{
//			hm = h;
//			break;
//		}
//	}
//	if (!hm)
//		return wstring();
//
//	//if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
//	//	GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
//	//	(LPCSTR)&functionInThisDll, &hm) == 0)
//	//{
//	//	int ret = GetLastError();
//	//	fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
//	//	// Return or however you want to handle an error.
//	//}
//	if (GetModuleFileName(hm, path, sizeof(path)) == 0)
//	{
//		// int ret = GetLastError();
//		// fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
//		// Return or however you want to handle an error.
//		return wstring();
//	}
//	return path;
//}

vector<wstring> GetTryPaths(LPCWSTR pFilePath)
{
	vector<wstring> tries;
	wstring ext = stdGetFileExtension(pFilePath);
	if (ext == L".com" || ext == L".exe" || ext == L".bat")
	{
		tries.push_back(pFilePath);
	}
	else
	{
		tries.push_back(pFilePath);
		tries.push_back(pFilePath + wstring(L".com"));
		tries.push_back(pFilePath + wstring(L".exe"));
		tries.push_back(pFilePath + wstring(L".bat"));
	}
	return tries;
}


int GetSubsystemFromImage(LPCWSTR pFilePath)
{
	wstring realPath;
	if (stdFileExists(pFilePath))
		realPath = pFilePath;
	else if(stdIsFullPath(pFilePath))
	{
		for (auto&& path : GetTryPaths(pFilePath))
		{
			if (stdFileExists(path.c_str()))
			{
				realPath = path;
				break;
			}
		}
	}
	else
	{
		for (auto&& path : GetTryPaths(pFilePath))
		{
			realPath = stdGetFullPathExecutable(path);
			if (!realPath.empty())
				break;
		}
	}
	if (realPath.empty())
		realPath = pFilePath;

	CKernelHandle file;
	DWORD dwCoffHeaderOffset;
	DWORD dwNewOffset;
	DWORD dwMoreDosHeader[16];
	ULONG ulNTSignature;

	IMAGE_DOS_HEADER dos_header;
	IMAGE_FILE_HEADER file_header;
	IMAGE_OPTIONAL_HEADER optional_header;

	static_assert(offsetof(IMAGE_OPTIONAL_HEADER32, Subsystem) == offsetof(IMAGE_OPTIONAL_HEADER64, Subsystem),
		"Subsystem offset must be equal");
	

	// Open the application file.

	file = CreateFile(realPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (!file) {
		// AfxMessageBox(_T("Failed to open the aplication file!\n"));
		return -1;
	}

	// Read MS-Dos image header.

	if (!ReadFileBytes(file, &dos_header, sizeof(IMAGE_DOS_HEADER))) {
		// AfxMessageBox(_T("Failed to read file!\n"));
		return -1;
	}

	if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
		// AfxMessageBox(_T("Application failed to classify the file type!\n"));
		return -1;
	}

	// Read more MS-Dos header.

	if (!ReadFileBytes(file, dwMoreDosHeader, sizeof(dwMoreDosHeader))) {
		// AfxMessageBox(_T("Failed to read file!\n"));
		return -1;
	}

	// Move the file pointer to get the actual COFF header.

	dwNewOffset = SetFilePointer(file, dos_header.e_lfanew, NULL, FILE_BEGIN);
	dwCoffHeaderOffset = dwNewOffset + sizeof(ULONG);

	if (dwCoffHeaderOffset == 0xFFFFFFFF) {
		// AfxMessageBox(_T("Failed to move file pointer!\n"));
		return -1;
	}

	// Read NT signature of the file.
	if (!ReadFileBytes(file, &ulNTSignature, sizeof(ULONG))) {
		// AfxMessageBox(_T("Failed to read NT signature of file!\n"));
		return -1;
	}

	if (ulNTSignature != IMAGE_NT_SIGNATURE) {
		// AfxMessageBox(_T("Missing NT signature!\n"));
		return -1;
	}

	if (!ReadFileBytes(file, &file_header, IMAGE_SIZEOF_FILE_HEADER)) {
		// AfxMessageBox(_T("Failed to read file!\n"));
		return -1;
	}

	// Read the optional header of file.
#define IMAGE_SIZEOF_NT_OPTIONAL32_HEADER    224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER    240

	int IMAGE_SIZEOF_NT_OPTIONAL_HEADER;
	if (file_header.Machine == IMAGE_FILE_MACHINE_I386)
		IMAGE_SIZEOF_NT_OPTIONAL_HEADER = IMAGE_SIZEOF_NT_OPTIONAL32_HEADER;
	else
		IMAGE_SIZEOF_NT_OPTIONAL_HEADER = IMAGE_SIZEOF_NT_OPTIONAL64_HEADER;

	if (!ReadFileBytes(file, &optional_header, IMAGE_SIZEOF_NT_OPTIONAL_HEADER)) {
		// AfxMessageBox(_T("Failed to read file for optional header!\n"));
		return -1;
	}

	return optional_header.Subsystem;
}

bool isOption(const wstring& option)
{
	return !option.empty() && option[0] == L'/';
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
	bool bWaitForProcess = false;
	wstring laucharg;
	CCommandLineString cmdline(lpCmdLine);
	size_t startIndexOfTarget = 0;
	for (; startIndexOfTarget < cmdline.getCount(); ++startIndexOfTarget)
	{
		wstring line = cmdline.getArg(startIndexOfTarget);
		if (!isOption(line))
			break;

		if (line == L"/h" || line == L"/?")
		{
			MessageBox(NULL,
				getHelpString().c_str(),
				stdFormat(L"%s v%s", APPNAME, GetVersionStringFromResource(nullptr,3).c_str()).c_str(),
				MB_ICONINFORMATION);
			return 0;
		}
		else if (line == L"/sleep")
		{
			++startIndexOfTarget;
			sleepsec = stoi(cmdline.getArg(startIndexOfTarget));
		}
		else if (line == L"/w")
		{
			bWaitForProcess = true;
		}
		else
		{
			MessageBox(NULL,
				stdFormat(I18N(L"Unknown option:%s"), line.c_str()).c_str(),
				APPNAME,
				MB_ICONERROR);
			return -1;
		}
	}

	const wstring exe = cmdline.getArg(startIndexOfTarget);
	const wstring args = cmdline.subString(startIndexOfTarget + 1);
	vector<wstring> v = { exe };
	const wstring tartgetCommandLine = CCommandLineString::getCommandLine(v) + L" " + args;
	bool isGui = false;
	switch (GetSubsystemFromImage(exe.c_str()))
	{
		case IMAGE_SUBSYSTEM_UNKNOWN:
			break;
		case IMAGE_SUBSYSTEM_NATIVE:
			break;
		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
			isGui = true;
			break;
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
			break;
		case IMAGE_SUBSYSTEM_OS2_CUI:
			break;
		case IMAGE_SUBSYSTEM_POSIX_CUI:
			break;
		case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:
			break;
		case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
			isGui = true;
			break;
	}
	if (isGui)
	{
		wstring message = stdFormat(I18N(L"'%s' is GUI. Are you sure to continue?"),
			exe.c_str());
		if (IDYES != MessageBox(NULL, message.c_str(), APPNAME, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2))
			return 1;
	}

	int ret=0;
	DWORD dwLE = 0;
	{
		CKernelHandle process;
		if (!CreateProcessCommon(tartgetCommandLine.c_str(),
			NULL,
			TRUE,
			&dwLE,
			WaitProcess_InputIdle,
			10 * 1000,
			&process))
		{
			wstring message;
			message += L"Failed to launch: \r\n";
			message += laucharg.c_str();
			message += L"\r\n\r\n";
			message += GetLastErrorString(dwLE);

			MessageBox(NULL, message.c_str(), APPNAME, MB_ICONEXCLAMATION);

			return 1;
		}

		if (bWaitForProcess)
			WaitForSingleObject(process, INFINITE);
	}

	if (sleepsec > 0)
		Sleep(sleepsec * 1000);

	return (ret);
}

