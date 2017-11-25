#include <windows.h>
//#pragma comment(lib,"Q:/ToolKit/VMProtect/Lib/COFF/VMProtectSDK32.lib")
//#include "Q:/ToolKit/VMProtect/Examples/Visual C++/VMProtectSDK.h"
#pragma comment(lib,"Q:/ToolKit/WinLicense/WinlicenseSDK/Lib/COFF/WinlicenseSDK32.lib")
#pragma comment(lib,"Q:/ToolKit/WinLicense/WinlicenseSDK/Lib/COFF/SecureEngineSDK32.lib")
#include "Q:/ToolKit/WinLicense/WinlicenseSDK/Include/C/WinlicenseSDK.h"
#include "Q:/ToolKit/WinLicense/WinlicenseSDK/Include/C/SecureEngineSDK.h"
int main()
{
    DebugBreak();   //利用系统断点直接停到代码虚拟化入口
    VMStart();    //VMProtect SDK来做标记
    int testconst = 0x12345678;
    if (MessageBoxA(0,"whitch?","logic diff test",MB_YESNO) == IDYES)
    {
        char buf[16];
         testconst++;
        itoa(testconst,buf,16);
        MessageBoxA(0,testconst,"case 1",MB_YESNO);
    }else
    {
        char buf[16];
         testconst--;
        itoa(testconst,buf,16);
        MessageBoxA(0,buf,"case2",MB_OK);
    }
    VMEnd();
    DebugBreak();
    return 0;
}

