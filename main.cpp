#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <process.h>
#include "properties.h"
#include "resource.h"

#define EXTRACT_PATH "%temp%\\" ProjectName

#include "shellapi.h"

DWORD ExtractResource(const HINSTANCE hInstance, int resourceId, const char* fileName)
{
	try
	{
		HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA);
		if (hResource == NULL)
			return false;
		HGLOBAL hFileResource = LoadResource(hInstance, hResource);
		if (hFileResource == NULL)
			return false;

		LPVOID lpFile = LockResource(hFileResource);
		DWORD dwSize = SizeofResource(hInstance, hResource);

		
		for (size_t i = 0, l = strlen(fileName); i < l; ++i)
		{
			if (fileName[i] == '\\') {
				char dir[MAX_PATH];
				strncpy_s(dir, MAX_PATH, fileName, i);
				if (CreateDirectoryA(dir, NULL) == 0)
				{
					DWORD error = GetLastError();
					if (error != ERROR_ALREADY_EXISTS)
					{
						return error;
					}
				}
			}
		}


		HANDLE hFile = CreateFileA(fileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD dwByteWritten;
		WriteFile(hFile, lpFile, dwSize, &dwByteWritten, NULL);
		CloseHandle(hFile);
		FreeResource(hFileResource);
		return ERROR_SUCCESS;

	}
	catch (...)
	{
		return ERROR_CAN_NOT_COMPLETE;
	}
}

INT WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, char* cmdLine, INT cmdShow)
{

	char extractPath[MAX_PATH];
	DWORD retVal;
	if ((retVal = ExpandEnvironmentStringsA(EXTRACT_PATH, extractPath, sizeof(extractPath)) == 0) || retVal > MAX_PATH) {
		DWORD error = GetLastError();
		char buffer[128] = "";
		sprintf_s(buffer, sizeof(buffer), "ExpandEnvironmentStrings failed!\nErrorCode: %d", error);
		MessageBox(0, buffer, ProjectName, MB_OK | MB_ICONERROR);
		return error;
	}

	if (CreateDirectoryA(extractPath, NULL) == 0)
	{
		DWORD error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS)
		{
			char buffer[128] = "";
			sprintf_s(buffer, sizeof(buffer), "CreateDirectory for '%s' failed!\nErrorCode: %d", extractPath, error);
			MessageBox(0, buffer, ProjectName, MB_OK | MB_ICONERROR);
			return error;
		}
	}


	for (int i = 0; i < sizeof(Resources) / sizeof(Resources[0]); i++)
	{
		char fileName[MAX_PATH];
		sprintf_s(fileName, MAX_PATH, "%s\\%s", extractPath, Resources[i].FileName);
		DWORD error = ExtractResource(instance, Resources[i].ResourceId, fileName);
		if (error != ERROR_SUCCESS)
		{
			char buffer[128] = "";
			sprintf_s(buffer, sizeof(buffer), "Extracting for '%s' failed!\nErrorCode: %d", Resources[i].FileName, error);
			MessageBox(0, buffer, ProjectName, MB_OK | MB_ICONERROR);
			return error;
		}
	}

	
	char binPath[MAX_PATH];
	sprintf_s(binPath, MAX_PATH, "%s\\" TargetName "" TargetExt, extractPath);
	return (int)_spawnl(P_OVERLAY | _P_NOWAIT, binPath, GetCommandLineA(), NULL);
}