#include <windows.h>
#include "internal.h"
#include "codeblock.h"

cb_SeqExecute* gLastSeq = 0;
int		grunState = bkrun_none;
int		gtmpStep = 0;
longptr glstBPAddr = 0;
longptr	glpContext = 0;
//维护最后100个内存访问记录
longptr	gwtedval[100];
int		gnwval = 0;
void detectOpBranch(void* cpu,cb_SeqExecute* se)
{
	//这里只是为了演示，我们如果发现访问记录在xor代码块的36个之间我们进行下一步判断
	if (gnwval >= 34 && gnwval <= 36)
	{	//如果倒数第5个记录像eflag我们认为为xor,注意这是不准确的，只是为了演示而已
		longptr efg = gwtedval[gnwval - 5];
		if ((efg & 0xFFFFFC00) == 0) //一般eflags前24位都为0
		{
			se->vars[0].type = cbvar_setcomment;
			strcpy(se->vars[0].byte,"xor");
			gnwval = 0;
		}
	}
}

void accessContextWrite(void* cpu,longptr rip,int oft,int size,longptr val)
{
	//addrprintf(rip,"WC:oft=%08X ->size:%d,Value:%08X",oft,size,val);
	if (size == 4 && gnwval < 100)
	{	//我们只记录100个以内的4字节写入指令
		gwtedval[gnwval++] = val;
	}
}


void accessContextRead(void* cpu,longptr rip,int oft,int size,longptr val)
{

}

int vmexec_before_step(void* cpu,void* rip,void* inst)
{
	//确保我们的调试状态是最新的
	grunState = vmexec_getRunState(cpu);
	//得到最后一个块执行记录
	cb_SeqExecute* se = vmexec_lastBKExec(cpu);
	if (se != gLastSeq)
	{	//说明已经转到了下一个代码块，我们进行上一个代码块的最后鉴别
		detectOpBranch(cpu,gLastSeq);
	}
	gLastSeq = se;
	if (se && se->rip == (longptr)rip)	//判断是否是代码块入口
	{
		gnwval = 0;	//每次代码入口重置写入记录
		if (gtmpStep == 0)
		{	
			if (grunState == bkrun_stepover)
				gtmpStep = 1;
		} else
		{	
			if (gtmpStep == 1)
			{
				gtmpStep = 0;
				se->flags |= cbseq_flag_temp;
				return vmexec_ret_finished;
			}
		}
		if (glpContext == 0)
			glpContext = vmexec_getReg(cpu,VM_REG_RBP);
		longptr* optr = (longptr*)(glpContext + 0x23);	//+0x51为opcode,每个加密可能不一样
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
		longptr val = glpContext;
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