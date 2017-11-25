#include <windows.h>
#include "internal.h"
//利用虚拟机进行实时监控分析

//单条指令执行前回调
int		vmexec_before_step(void* cpu,void* rip,void* inst)
{	//我们简单的判断是否ripv在系统代码内，如果是我们就暂停追踪
	int oplen;
	char* lpop = vmexec_getInstOpByte(inst,&oplen);
	if (!lpop)
		return vmexec_ret_normal;
	if ((unsigned char)*lpop == 0xE8)
	{
		addrprintf((unsigned long)rip,"call->target:%08X",(unsigned long)rip+*(long*)(lpop+1)+5);
	}
	return vmexec_ret_normal;
}
//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}