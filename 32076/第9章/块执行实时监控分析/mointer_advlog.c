#include <windows.h>
#include "internal.h"
#include "codeblock.h"

cb_SeqExecute* gLastSeq = 0;

void accessContextRead(void* cpu,longptr rip,int oft,int size,longptr val)
{
	if (!gLastSeq) return;
	addrprintf(rip,"%d",gLastSeq->uuid);
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
	case 2:// push %02s->%08s
	{
			   gLastSeq->vars[0].type = cbvar_int;
			   gLastSeq->vars[0].sqword = oft;
			   gLastSeq->vars[1].type = cbvar_int;
			   gLastSeq->vars[1].sqword = val;
	}break;
	default:	addrprintf(rip,"%d",gLastSeq->uuid); break;
	}

}


void accessVStackRead(void* cpu,longptr rip,longptr stkaddr,int size,longptr val)
{
	if (!gLastSeq) return;
	addrprintf(rip,"%d",gLastSeq->uuid);
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
	default:	addrprintf(rip,"%d",gLastSeq->uuid); break;
	}
}



//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	cb_SeqExecute* se = vmexec_lastBKExec(cpu);
	if (se && se->rip == (longptr)rip)	//判断是否是代码块入口
	{
		se->ip = vmexec_getReg(cpu,VM_REG_RSI) + 1;	//+1修正
		gLastSeq = se;
	}
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
				accessContextRead(cpu,(longptr)rip,ctxoftr,szrmem,mrval);
		}
		//这里我们监控对虚拟栈的访问
		valr = vmexec_getReg(cpu,VM_REG_RBP);
		if (lprmem >= valr)
		{	//如果访问发生在Context内
			longptr ctxoftr = lprmem - valr;
			//计算偏移，如果偏移小于0xA0我们认为是有效的Context访问
			if (ctxoftr < 0x20)
				accessVStackRead(cpu,(longptr)rip,lprmem,szrmem,mrval);
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
				accessContextWrite(cpu,(longptr)rip,ctxoftw,szwmem,mwval);
		}
		//这里我们监控对虚拟栈的访问
		valw = vmexec_getReg(cpu,VM_REG_RBP);
		if (lpwmem >= valw)
		{	//如果访问发生在Context内
			longptr ctxoftw = lpwmem - valw;
			//计算偏移，如果偏移小于0xA0我们认为是有效的Context访问
			if (ctxoftw < 0x20)
				accessVStackWrite(cpu,(longptr)rip,lpwmem,szwmem,mwval);
		}
	}

	//这里我们使用高级调试断点技术来暂停在下一个OP代码分支的入口
	int szop;
	char* lpop = vmexec_getInstOpByte(inst,&szop);
	if (lpop)
	{	//我们如果发现ret 形式的跳转我们认为是OP代码分支结束
		if (*(unsigned char*)lpop == 0xC2)
			return vmexec_ret_finished;	//返回退出虚拟机指令
	}
	return vmexec_ret_normal;
}


int main(int argc,char** argv)
{	
	//这里我们返回全局脚本标记，这样脚本只会在第一次编译
	return main_ret_global;
}