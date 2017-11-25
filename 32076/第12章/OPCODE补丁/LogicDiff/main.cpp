#include <windows.h>
#pragma comment(lib,"Q:/ToolKit/VMProtect/Lib/COFF/VMProtectSDK32.lib")
#include "Q:/ToolKit/VMProtect/Include/C/VMProtectSDK.h"

int main()
{
    DebugBreak();   //利用系统断点直接停到代码虚拟化入口
    VMProtectBegin("Test");    //VMProtect SDK来做标记
    if (MessageBoxA(0,"正常情况选择“是”将出现OK:","选择",MB_YESNO)==IDYES)
    {
        MessageBoxA(0,"OK","OK",MB_OK|MB_ICONINFORMATION);
    }else
    {
        MessageBoxA(0,"ERROR","ERROR",MB_OK|MB_ICONERROR);
    }
    VMProtectEnd();
    DebugBreak();
    return 0;
}

