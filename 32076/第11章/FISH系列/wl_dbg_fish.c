#include <windows.h>
#include "internal.h"
#include "codeblock.h"

cb_SeqExecute* gLastSeq = 0;
int		grunState = bkrun_none;
int		gtmpStep = 0;
longptr glstBPAddr = 0;

void accessContextRead(void* cpu,longptr rip,int oft,int size,longptr val)
{

}


void accessContextWrite(void* cpu,longptr rip,int oft,int size,longptr val)
{
	addrprintf(rip,"WC:oft=%08X ->size:%d,Value:%08X",oft,size,val);
}

int vmexec_before_step(void* cpu,void* rip,void* inst)
{
	//确保我们的调试状态是最新的
	grunState = vmexec_getRunState(cpu);
	//得到最后一个块执行记录
	cb_SeqExecute* se = vmexec_lastBKExec(cpu);
	gLastSeq = se;
	//如果有块执行记录，说明代码执行已经进入块执行循环了
	if (se && se->rip == (longptr)rip)	//判断是否是代码块入口
	{
		//这里我们首先判断我们是否已经设定了步过标记，这个标记是在步过开始时设定的
		//因为我们只能在下一个代码块入口才能确定上一个代码块已经执行完毕了
		// 因此我们判断步过的标记是下一个代码块准备执行，那么我们认为步过了上一个代码块
		if (gtmpStep == 0)
		{	//如果没有而调试器又请求了步过调试那么我们设定标记
			if (grunState == bkrun_stepover)
				gtmpStep = 1;
		} else
		{	//这里我们检测到调试器请求步过调试，并且已经设定了标记，这里已经是下一个代码入口，
			//那么我们退出虚拟机，暂停执行
			if (gtmpStep == 1)
			{
				gtmpStep = 0;
				se->flags |= cbseq_flag_temp;
				return vmexec_ret_finished;
			}
		}
		//这里我们检测OPCODE断点，因为OPCODE指针我们调试器是无法自动识别的
		//因此判断是否触发断点由脚本自身完成
		longptr* optr = (longptr*)(vmexec_getReg(cpu,VM_REG_RBP)+0x54);	//+0x54为opcode,每个加密可能不一样
		se->ip = *optr;
		if (glstBPAddr != se->ip)
		{
			if (vmexec_isBreak(cpu,(void*)se->ip))
			{
				glstBPAddr = se->ip;
				return vmexec_ret_finished;
			}
		}
	}
	glstBPAddr = 0;

	return vmexec_ret_normal;
}
//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	vaddr lpmem;
	int szmem;//我们获取最后一条指令的内存访问信息
	longptr memval;
	int mode = vmexec_getLastMemAcc(cpu,FALSE,&lpmem,&szmem,&memval);
	if (mode != 0)
	{		//根据访问地址过滤并记录
		//__asm__("int3");	//我们的脚本支持汇编，可以用于调试暂停
		//fish32使用RBP来指向Context,其它寄存器都是任意使用的
		longptr val = vmexec_getReg(cpu,VM_REG_RBP);
		//由于Context没有在栈当中，因为我们假设有0x1000大小也不为过
		if (lpmem >= val && lpmem <= val + 0x1000)
		{	//如果访问发生在Context内
			longptr ctxoft = lpmem - val;
			//计算偏移，如果偏移小于0x1000我们认为是有效的Context访问
			if (ctxoft < 0x1000)
			{	
				if (mode & memacc_flag_read)
				{
					accessContextRead(cpu,(longptr)rip,ctxoft,szmem,memval);
				} else if (mode & memacc_flag_write)
				{
					accessContextWrite(cpu,(longptr)rip,ctxoft,szmem,memval);
				}

			}
		}
	}
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_global;
}