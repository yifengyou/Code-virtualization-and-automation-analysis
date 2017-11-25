#include <windows.h>
#include "internal.h"
#include "codeblock.h"

//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{

	cb_SeqExecute* se = vmexec_lastBKExec(cpu);
	if (se && se->rip == (longptr)rip)	//判断是否是代码块入口
		se->ip = vmexec_getReg(cpu,VM_REG_RSI)+1;	//+1修正

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