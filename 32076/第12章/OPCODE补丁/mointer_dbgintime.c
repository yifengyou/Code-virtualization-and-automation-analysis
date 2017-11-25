#include <windows.h>
#include "internal.h"
#include "codeblock.h"

cb_SeqExecute* gLastSeq = 0;
int		grunState = bkrun_none;
int		gtmpStep = 0;
longptr glstBPAddr = 0;
void accessContextRead(void* cpu,longptr rip,int oft,int size,longptr val)
{
	//这里我们看是否有最后一个快执行记录
	if (!gLastSeq) return;
	//这里我们分OP分支处理
	switch (gLastSeq->uuid)
	{
	case 4:// push %02s->%08s
	{
			   gLastSeq->vars[0].type = cbvar_int;
			   gLastSeq->vars[0].sqword = oft;
			   gLastSeq->vars[1].type = cbvar_int;
			   gLastSeq->vars[1].sqword = val;
	}break;
	default: break;
	}
}


void accessContextWrite(void* cpu,longptr rip,int oft,int size,longptr val)
{
	//这里我们看是否有最后一个快执行记录
	if (!gLastSeq) return;
	//这里我们分OP分支处理
	switch (gLastSeq->uuid)
	{
	case 1:	//pop %1	
	{	//如果是pop代码分支，那么我们将数据通过传输传递给调试插件
		gLastSeq->vars[0].type = cbvar_int;
		gLastSeq->vars[0].sqword = oft;
		gLastSeq->vars[1].type = cbvar_int;
		gLastSeq->vars[1].sqword = val;
	}break;
	case 4:// push %02s->%08s
	{
		gLastSeq->vars[0].type = cbvar_int;
		gLastSeq->vars[0].sqword = oft;
		gLastSeq->vars[1].type = cbvar_int;
		gLastSeq->vars[1].sqword = val;
	}break;
	default:break;
	}

}


void accessVStackRead(void* cpu,longptr rip,longptr stkaddr,int size,longptr val)
{
	if (!gLastSeq) return;
	switch (gLastSeq->uuid)
	{
	case 3:	//add [esp+4],[esp] <- eflags
	{	
		gLastSeq->vars[1].type = cbvar_int;
		gLastSeq->vars[1].sqword = val;
	}break;
	case 5:
	{
		gLastSeq->vars[1].type = cbvar_int;
		gLastSeq->vars[1].sqword = val;
	}break;
	default:break;
	}
}

void accessVStackWrite(void* cpu,longptr rip,longptr stkaddr,int size,longptr val)
{
	if (!gLastSeq) return;
	switch (gLastSeq->uuid)
	{
	case 2:	//push imm
	{	//同样分OP分支处理，因为push imm在栈写入上监控最准确
		gLastSeq->vars[0].type = cbvar_int;
		gLastSeq->vars[0].sqword = val;
	}break;
	case 3:
	{
		longptr vstk = vmexec_getReg(cpu,VM_REG_RBP);
		longptr oft = stkaddr - vstk;
		if (oft == 0)
		{
			gLastSeq->vars[2].type = cbvar_int;
			gLastSeq->vars[2].sqword = val;
		} else if (oft == 4)
		{
			gLastSeq->vars[0].type = cbvar_int;
			gLastSeq->vars[0].sqword = val;
		}
	}break;
	case 5:
	{
		gLastSeq->vars[0].type = cbvar_int;
		gLastSeq->vars[0].sqword = val;
	}break;
	default:break;
	}
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
		se->ip = vmexec_getReg(cpu,VM_REG_RSI) + 1;	//+1修正
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
	vaddr lprmem;
	int szrmem;
	longptr mrval;
	int mode = vmexec_getLastMemAcc(cpu,TRUE,&lprmem,&szrmem,&mrval);
	if (mode != 0)
	{
		//我们先处理只读取访问
		longptr valr = vmexec_getReg(cpu,VM_REG_RDI);
		//由于Context没有在栈当中，因为我们假设有0x1000大小也不为过
		if (lprmem >= valr)
		{	//如果访问发生在Context内
			longptr ctxoftr = lprmem - valr;
			//计算偏移，如果偏移小于0xA0我们认为是有效的Context访问
			if (ctxoftr < 0xA0)
			{
				accessContextRead(cpu,(longptr)rip,ctxoftr,szrmem,mrval);
				if (grunState == bkrun_stepin)
				{
					grunState = bkrun_none;
					return vmexec_ret_finished;
				}
			}	
		}
		//这里我们监控对虚拟栈的访问
		valr = vmexec_getReg(cpu,VM_REG_RBP);
		if (lprmem >= valr)
		{	//如果访问发生在Context内
			longptr ctxoftr = lprmem - valr;
			//计算偏移，如果偏移小于0xA0我们认为是有效的Context访问
			if (ctxoftr < 0x20)
			{
				accessVStackRead(cpu,(longptr)rip,lprmem,szrmem,mrval);
				if (grunState == bkrun_stepin)
				{
					grunState = bkrun_none;
					return vmexec_ret_finished;
				}
			}	
		}
	}

	vaddr lpwmem;
	int szwmem;//我们获取最后一条指令的内存访问信息
	longptr mwval;
	mode = vmexec_getLastMemAcc(cpu,FALSE,&lpwmem,&szwmem,&mwval);
	if (mode != 0)
	{		//根据访问地址过滤并记录
		//__asm__("int3");	//我们的脚本支持汇编，可以用于调试暂停
		//该该程式中EDI指向Context,ESI指向OPCODE,EBP指向vStack
		longptr valw = vmexec_getReg(cpu,VM_REG_RDI);
		//由于Context没有在栈当中，因为我们假设有0x1000大小也不为过
		if (lpwmem >= valw)
		{	//如果访问发生在Context内
			longptr ctxoftw = lpwmem - valw;
			//计算偏移，如果偏移小于0x1000我们认为是有效的Context访问
			if (ctxoftw < 0xA0)
			{
				accessContextWrite(cpu,(longptr)rip,ctxoftw,szwmem,mwval);
				if (grunState == bkrun_stepin)
				{
					grunState = bkrun_none;
					return vmexec_ret_finished;
				}
			}
		}
		//这里我们监控对虚拟栈的访问
		valw = vmexec_getReg(cpu,VM_REG_RBP);
		if (lpwmem >= valw)
		{	//如果访问发生在Context内
			longptr ctxoftw = lpwmem - valw;
			//计算偏移，如果偏移小于0xA0我们认为是有效的Context访问
			if (ctxoftw < 0x20)
			{
				accessVStackWrite(cpu,(longptr)rip,lpwmem,szwmem,mwval);
				if (grunState == bkrun_stepin)
				{
					grunState = bkrun_none;
					return vmexec_ret_finished;
				}
			}
		}
	}
	return vmexec_ret_normal;
}


int main(int argc,char** argv)
{	
	grunState = bkrun_none;
	//这里我们返回全局脚本标记，这样脚本只会在第一次编译
	return main_ret_global;
}