// start.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <atlstr.h>

typedef void (*BEGDIALOGHOOK)();
typedef void (*ENDDIALOGHOOK)();

int get_current_dir(CString &path)
{
	TCHAR temp[MAX_PATH] = {0};
	DWORD dwret = GetModuleFileName(NULL, temp, MAX_PATH);
	if (0 == dwret)
	{
		return -1;
	}

	TCHAR drive[_MAX_DRIVE] = { 0 };
	TCHAR dir[_MAX_DIR] = { 0 };
	TCHAR file[_MAX_FNAME] = { 0 };
	TCHAR ext[_MAX_EXT] = { 0 };
	errno_t err = 0;
	err = _tsplitpath_s(temp, drive, _MAX_DRIVE, dir, _MAX_DIR,
		file, _MAX_FNAME, ext, _MAX_EXT);
	if (0 != err)
	{
		return -1;
	}

	path += drive;
	path += dir;

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	CString str_exe;
	get_current_dir(str_exe);

	str_exe.Append(L"mfc_dialog.exe");
	if (-1 == _taccess(str_exe, 0))
	{
		printf("mfc_dialog.exe not found\n");
		return -1;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	if (!CreateProcess(NULL, str_exe.GetBuffer(), NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
	{
		printf("CreateProcess failed: %d\n", GetLastError());
		return -1;
	}

	HMODULE hmodule = LoadLibrary(L"hook_mfc_dialog");
	if (NULL == hmodule)
	{
		printf("LoadLibrary failed: %d\n", GetLastError());
		CloseHandle(pi.hProcess);
		return -1;
	}

	BEGDIALOGHOOK beg_hook = (BEGDIALOGHOOK)GetProcAddress(hmodule, "BegDialogHook");
	if (NULL == beg_hook)
	{
		printf("GetProcAddress BegDialogHook failed: %d\n", GetLastError());
		CloseHandle(pi.hProcess);
		FreeLibrary(hmodule);
		return -1;
	}

	ENDDIALOGHOOK end_hook = (ENDDIALOGHOOK)GetProcAddress(hmodule, "EndDialogHook");
	if (NULL == end_hook)
	{
		printf("GetProcAddress EndDialogHook failed: %d\n", GetLastError());
		CloseHandle(pi.hProcess);
		FreeLibrary(hmodule);
		return -1;
	}

	WaitForSingleObject(pi.hProcess, 2000);

	beg_hook();
	getchar();
	end_hook();

	CloseHandle(pi.hProcess);
	FreeLibrary(hmodule);
	return 0;
}

