
#include <windows.h>

#pragma comment(lib,"Q:/ToolKit/WinLicense/WinlicenseSDK/Lib/COFF/WinlicenseSDK32.lib")
#pragma comment(lib,"Q:/ToolKit/WinLicense/WinlicenseSDK/Lib/COFF/SecureEngineSDK32.lib")
#include "Q:/ToolKit/WinLicense/WinlicenseSDK/Include/C/WinlicenseSDK.h"
#include "Q:/ToolKit/WinLicense/WinlicenseSDK/Include/C/SecureEngineSDK.h"
int main()
{
    DebugBreak();
    VM_START
        __asm {
            mov edi,0x12345678
lab_again:
            push edi	//因为Winlicense是平栈的，因此我们通过连续的压栈来暴露代码
            xor eax,eax		//xor指令就是我们的目标指令，我们想搞清楚那个代码分支负责处理这个指令
            pop edi		//通过栈变化来定位我们代码的开始和结束
            jmp lab_again
    }
    VM_END
        DebugBreak();
    return 0;
}
