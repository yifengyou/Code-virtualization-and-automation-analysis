#include <windows.h>
#include "internal.h"
//利用虚拟机进行实时监控分析
//监控寄存器访问脚本

//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	vaddr lpmem;
	int szmem;//我们获取最后一条指令的内存访问信息
	longptr memval;
	int mode = vmexec_getLastMemAcc(cpu,FALSE,&lpmem,&szmem,&memval);
	if (mode != 0 && !(mode & memacc_flag_stackwrite))
	{		//根据访问地址过滤并记录
		//__asm__("int3");	//我们的脚本支持汇编，可以用于调试暂停
		//fish32使用RBP来指向Context,其它寄存器都是任意使用的
		longptr val = vmexec_getReg(cpu,VM_REG_RBP);
		//由于Context没有在栈当中，因为我们假设有0x1000大小也不为过
		if (lpmem >= val && lpmem <= val+0x1000)
		{	//如果访问发生在Context内
			longptr ctxoft = lpmem - val;
			//计算偏移，如果偏移小于0x1000我们认为是有效的Context访问
			if (ctxoft < 0x1000)
			{	//我们将访问信息发送到调试器的日志窗口
				addrprintf((long)rip,"[%c]AC:%08X->oft=%08X ->Value:%08X",(mode & memacc_flag_read)?'R':'W', lpmem,ctxoft,memval);
			}
		}
	}
	//这里我们使用高级调试断点技术来暂停在下一个OP代码分支的入口
	int szop;
	char* lpop = vmexec_getInstOpByte(inst,&szop);
	if (lpop && szop == 2)
	{	//我们如果发现jmp reg形式的跳转我们认为是OP代码分支结束
		unsigned char tp2 = *(unsigned char*)(lpop + 1);
		if (*(unsigned char*)lpop == 0xFF && tp2 >= 0xE0 && tp2 <= 0xE7)
			return vmexec_ret_finished;	//返回退出虚拟机指令
	}

	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}