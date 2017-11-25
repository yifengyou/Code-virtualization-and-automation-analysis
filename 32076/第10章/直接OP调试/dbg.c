#include <windows.h>
#include "internal.h"
#include "codeblock.h"
#include "plugin.h"	//ollydbg2的PDK头文件，没仔细看分发许可，因此随书文件没提供

int	gseqID = -1;
void* CALLBACK opcode_pass(xhook_regs* stregs,void* orgEntry,void* userdata)
{
	cb_SeqExecute se;
	struct opcode_* op;
	memset(&se,0,sizeof(se));
	
	se.seqid = gseqID;
	se.ip = stregs->rsi+1;	//取得opcode地址
	//发送代码块执行记录
	cb_send_execlog(&se);
	//这里我们检测是否有代码块单步或者断点触发
	if (cb_getRunState() == bkrun_stepover || cb_isBreak(se.ip) != 0)
	{	//如果有断点，那么我们直接通过调试器的断点函数在HOOK程式的出口上设定断点
		//当然我们也可以通过自动分析暂停到代码块的真实入口或者其它任何地方
		dbg_Setint3breakpoint(orgEntry,BP_ONESHOT | BP_EXEC | BP_BREAK,0,0,0,0,0,0);
	}

	return orgEntry;
}
int main(int argc,char** argv)
{	
	//先将调试模式设定为直接调试模式
	cb_set_debugmode(cb_dbg_direct);
	//我们直接通过脚本的代码HOOK函数在目标程式的代码分支跳转上HOOK
	hook_hookCode((void*)0x119B559,opcode_pass,hook_callback,0);
	//我们新建一个代码块用于演示，实际上在实际过程中我们可以通过手动整理精确的代码块
	gseqID = cb_make_codeblock(1,0x119B559,-1,0,L"dummy",0);
	//这里我们返回全局脚本标记，这样脚本会一直驻留内存而不会被释放
	return main_ret_global;
}