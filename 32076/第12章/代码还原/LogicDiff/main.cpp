#include <windows.h>
#pragma comment(lib,"Q:/ToolKit/VMProtect/Lib/COFF/VMProtectSDK32.lib")
#include "Q:/ToolKit/VMProtect/Include/C/VMProtectSDK.h"

int main()
{
    DebugBreak();   //利用系统断点直接停到代码虚拟化入口
    VMProtectBegin("Test");    //VMProtect SDK来做标记
    __asm {
        mov ecx,0xE0001234
        mov ebx,0xF0004567
lab_again:
        mov edi,0x12121212//我们通过连续的压栈来暴露代码
        xor ecx,ebx		//xor指令就是我们的目标指令，我们想搞清楚哪些代码分支负责处理这个指令
        mov edi,0x34343434
        jmp lab_again
}
    VMProtectEnd();
    DebugBreak();
    return 0;
}

