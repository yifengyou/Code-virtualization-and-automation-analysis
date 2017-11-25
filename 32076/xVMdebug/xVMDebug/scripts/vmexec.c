#include <windows.h>
#include "internal.h"
//利用虚拟机进行高级自动追踪分析例子

//单条指令执行前回调
int		vmexec_before_step(void* cpu,void* rip,void* inst)
{
	int len = 0;
	unsigned char* lpop = (unsigned char*)vmexec_getInstOpByte(inst,&len);
	if (lpop && len == 2)
	{
		if (lpop[0] == 0xFF && lpop[1] >= 0xE0 && lpop[1] <= 0xE7)
		{	//jmp reg
			return vmexec_ret_finished;
		}
	}
	return vmexec_ret_normal;
}
//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{

	return vmexec_ret_normal;
}
//主体循环前回调
//int		vmexec_before_loop(void* cpu,int reason)
//{
//	MessageBoxA(0,"bl","",MB_OK);
//	return reason;
//}
//主体循环后回调
//int		vmexec_after_loop(void* cpu,int reason)
//{
//	MessageBoxA(0,"al","",MB_OK);
//	return reason;
//}

int main(int argc,char** argv)
{

	
	return main_ret_keep;
}