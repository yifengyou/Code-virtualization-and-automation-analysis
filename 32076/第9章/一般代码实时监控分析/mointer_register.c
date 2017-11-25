#include <windows.h>
#include "internal.h"
//利用虚拟机进行实时监控分析
//监控寄存器访问脚本

//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	longptr val = vmexec_getReg(cpu,VM_REG_RAX);
	if (val == 0xBB40E64E)
	{
		addrprintf((long)rip,"EAX=%08X",val);
	}
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}