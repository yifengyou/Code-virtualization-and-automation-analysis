#include <windows.h>
#include "internal.h"
typedef LONG(NTAPI* LPZwProtectVirtualMemory)(
	HANDLE hProcess,
	PVOID *BaseAddress,
	PULONG RegionSize,
	ULONG Protect,
	PULONG OldProtect
	);


typedef BOOL (WINAPI* LPCreatePipe)(
PHANDLE hReadPipe,
PHANDLE hWritePipe,
LPSECURITY_ATTRIBUTES lpPipeAttributes,
DWORD nSize
);

void VirtualLoadModule(HMODULE hModule)
{
	if (pe_isValidPE((const char*)hModule) == 0) return;
	char lpfn[MAX_PATH];
	int nlen = GetModuleFileNameA(hModule,lpfn,sizeof(lpfn));
	if (nlen > 0)
		return;	//如果模块是有效的，但是没有出现在进程模块列表中
	const char* lpImage = (const char*)hModule;
	PIMAGE_NT_HEADERS imnh = (PIMAGE_NT_HEADERS)(lpImage + ((PIMAGE_DOS_HEADER)lpImage)->e_lfanew);
	IMAGE_DATA_DIRECTORY* imdtexp = &imnh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (imdtexp->VirtualAddress == 0 || imdtexp->Size == 0)
		return;
	//验证导出表
	PIMAGE_EXPORT_DIRECTORY imed = (PIMAGE_EXPORT_DIRECTORY)(lpImage + imdtexp->VirtualAddress);
	if (IsBadReadPtr(imed,1)) return;
	if (!imed->Name)
		return;
	if (*(BYTE*)((ULONG_PTR)hModule+0x1000) != 0x55)
		return;
	//如果有导出，那么我们自行虚拟载入
	const char* lpName = lpImage + imed->Name;
	wchar_t	fname[MAX_PATH];
	wcscpy(fname,L"Q:\\RD\\wl\\23t\\");
	int ns = strlen(lpName);
	int pos = 0;
	nlen = wcslen(fname);
	for (; pos < ns; pos++)
		fname[nlen++] = lpName[pos];
	fname[nlen++] = 0;	//整理路径并发送日志
	printf("Loading Virtual Module:%08X -> %S",hModule,fname);
	//通过扩展函数来通知调试器
	dbg_raiseLoadDllException(hModule,fname);
}

LPZwProtectVirtualMemory glpZwProtectVirtualMemory = 0;
LONG NTAPI Hooked_ZwProtectVirtualMemory(
	HANDLE hProcess,
	PVOID *BaseAddress,
	PULONG RegionSize,
	ULONG Protect,
	PULONG OldProtect
	)
{
	if (BaseAddress)
	{
		ULONG_PTR upaddr = (ULONG_PTR)*BaseAddress;
		if ((upaddr & 0xFFFF) == 0)
		{
			if (!IsBadReadPtr((const VOID*)upaddr,2))
			{
				if (*(WORD*)upaddr == 0x5A4D)
				{
					VirtualLoadModule((HMODULE)upaddr);
				}
			}
		}
	}
	return glpZwProtectVirtualMemory(hProcess,BaseAddress,RegionSize,Protect,OldProtect);
}

LPCreatePipe glpCreatePipe = 0;
BOOL WINAPI Hooked_CreatePipe(
	PHANDLE hReadPipe,
	PHANDLE hWritePipe,
	LPSECURITY_ATTRIBUTES lpPipeAttributes,
	DWORD nSize
	)
{
	if (nSize == 0 && lpPipeAttributes == 0) return FALSE;
	return glpCreatePipe(hReadPipe,hWritePipe,lpPipeAttributes,nSize);
}


int main(int argc,char **argv)
{
	HMODULE hNtdll;
	DWORD oldpg;
	glpCreatePipe = (LPCreatePipe)hook_hookApi(L"KERNELBASE.DLL","CreatePipe",Hooked_CreatePipe,hook_push);
	if (!glpZwProtectVirtualMemory)	//挂接ZwProtectVirtualMemory函数
	{
		hNtdll = GetModuleHandleA("NTDLL");
		void* lpFunc = GetProcAddress(hNtdll,"ZwProtectVirtualMemory");
		if (VirtualProtect(lpFunc,16,PAGE_EXECUTE_READWRITE,&oldpg))
		{
			glpZwProtectVirtualMemory = (LPZwProtectVirtualMemory)hook_hookCodeDirect(hNtdll,lpFunc,Hooked_ZwProtectVirtualMemory,hook_push,0);
			VirtualProtect(lpFunc,16,oldpg,&oldpg);
		}	
	}
	return main_ret_keep;
}