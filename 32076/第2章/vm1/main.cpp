#include <windows.h>


int main()
{
    DWORD dwval;
    __asm{
        mov eax,0x10000000
        add eax,0x1234
        mov dwval,eax
    }
    char buf[16];
    itoa(dwval,buf,16);
    MessageBoxA(0,buf,"Result:",MB_OK|MB_ICONINFORMATION);
    return 0;
}

