// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

extern void initPipeServer();
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		TCHAR maxpath[MAX_PATH];
		GetModuleFileName(hModule,maxpath,sizeof(maxpath) / sizeof(TCHAR));
		LoadLibrary(maxpath);
		initPipeServer();
	}break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

