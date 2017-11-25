#include <windows.h>
#include "internal.h"
//利用虚拟机进行实时监控分析
//监控内存访问脚本

//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	vaddr lpmem;
	int szmem;//我们获取最后一条指令的内存访问信息
	int mode = vmexec_getLastMemAcc(cpu,&lpmem,&szmem,0);
	if (mode != 0)
	{		//根据访问地址过滤并记录
		if (lpmem >= 0x408000 && lpmem <= 0x410000)
		{
			addrprintf((long)rip,"acc MEM:%08X",lpmem);
		}
	}
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}