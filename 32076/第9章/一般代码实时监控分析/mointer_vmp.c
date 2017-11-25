#include <windows.h>
#include "internal.h"
//利用虚拟机进行实时监控分析
//监控寄存器访问脚本

//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	vaddr lpmem;
	int szmem;//我们获取最后一条指令的内存访问信息
	int mode = vmexec_getLastMemAcc(cpu,&lpmem,&szmem,0);
	if (mode != 0 && !(mode & memacc_flag_stack))
	{		//根据访问地址过滤并记录
		//__asm__("int3");
		longptr val = vmexec_getReg(cpu,VM_REG_RDI);
		if (lpmem >= val && lpmem <= val+0x100)
		{
			longptr val2 = vmexec_getReg(cpu,VM_REG_RAX);
			if (val2 >= 0xC && val2 < 0x1000)
			{
				addrprintf((long)rip,"Access to Context:%08X->eax=%08X",lpmem,val2);
			}
		}
	}
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}