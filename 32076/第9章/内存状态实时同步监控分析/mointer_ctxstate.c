#include <windows.h>
#include "internal.h"
#include "xstate.h"
//利用虚拟机进行实时监控分析
//内存状态实时同步监控分析

void*	gxstate = 0;//对Context的追踪状态
void*	gvstkst = 0;//对虚拟栈的追踪状态
int		gmemberid = 1;

//这里我们虚拟一个成员ID栈出来，可以临时存放16个成员ID
#define max_nstackid	16
int		gmidstack[max_nstackid];
//栈中拥有的成员实际数量
int		gnmid = 0;
//表示成员已经被访问过了
#define mid_state_dirty	0x10000

int		glastBlockID = -1;
//将成员压入栈
void pushMemberToStack(int mid)
{
	//我们这里将成员访问压入成员ID栈
	if (gnmid >= max_nstackid)
	{
		gnmid = max_nstackid;
		//超出了限制,删除最老的一个，当然也可打印bug
		addrprintf(0,"MayBe Bug");
		for (int i = 1; i < gnmid; i++)
			gmidstack[i - 1] = gmidstack[i];
		gnmid--;
	}
	gmidstack[gnmid++] = mid;
}
//在4个操作当中更新并判断当前的代码块
int updateAndCheckLastBlock(void* cpu)
{
	int blockID = vmexec_lastBlockID(cpu);
	if (glastBlockID != blockID)
	{
		glastBlockID = blockID;
		return 0;
	}
	return 1;
}
void accessContextRead(void* cpu,longptr rip,int oft,int size)
{
	int mid;
	int voft = oft;
	//更新并判断是否是最后一个代码块
	int isSameBlock = updateAndCheckLastBlock(cpu);
	//这里我们从状态管理器当中去当前Context偏移范围是否有状态
	int szhit = xst_hit(gxstate,&voft,size,&mid);
	if (szhit == 0)
	{
		//如果范围内没有着色信息，说明我们的追踪有遗漏
		addrprintf(rip,"MayBe Bug,Because Got rid of our trace:oft:%08X ->size:%d",oft,size);
	} else
	{
		mid &= ~mid_state_dirty;
		addrprintf(rip,"[%d]var%d <- oft:%08X ->size:%d",glastBlockID,mid,oft,size);
		pushMemberToStack(mid);
		//这里我们将Context的对应状态修改为已经读取过
		xst_set(gxstate,voft,size,mid | mid_state_dirty);
	}
}


void accessContextWrite(void* cpu,longptr rip,int oft,int size)
{
	int isSameBlock = updateAndCheckLastBlock(cpu);
	if (gnmid > 0)
	{
		//如果成员ID栈里面有成员，那么我们当成成员迁移来处理
		if (isSameBlock)
		{	//如果还在同一个OP分支里面，我们才处理
			gnmid--;
			int mid = gmidstack[gnmid];
			mid &= ~mid_state_dirty;
			xst_set(gxstate,oft,size,mid);
			addrprintf(rip,"var%d -> oft:%08X->size:%d",mid,oft,size);
		}
	} else
	{
		//如果该空间没有状态，说明有迁移我们要么试图找到对应的变量关系
		//要么新建一个状态ID
		int mid = gmemberid++;
		xst_set(gxstate,oft,size,mid);
		addrprintf(rip,"[%d]:new var%d -> oft=%08X ->size:%d",glastBlockID,mid,oft,size);
	}

}

void accessVStackRead(void* cpu,longptr rip,longptr stkaddr,int size)
{
	int mid;
	longptr voft = stkaddr;
	//更新并判断是否是最后一个代码块
	int isSameBlock = updateAndCheckLastBlock(cpu);
	//这里我们从状态管理器当中去当前Context偏移范围是否有状态
	int szhit = xst_hit(gvstkst,&voft,size,&mid);
	if (szhit != 0)
	{
		//如果我们检测到了成员信息存在于虚拟栈上那么我们将这个成员ID放入临时变量栈
		//我们这里将成员访问压入成员ID栈
		pushMemberToStack(mid);
		//这里我们将状态管理器中的对应状态修改为已经读取过
		xst_set(gvstkst,voft,size,mid | mid_state_dirty);
		//打印访问记录
		addrprintf(rip,"[%d]:var%d <- addr:%08X->size:%d",glastBlockID,mid & ~mid_state_dirty,stkaddr,size);
	}else
		addrprintf(rip,"[%d]:Vstack Read->addr:%08X->size:%d",glastBlockID,stkaddr,size);
}

void accessVStackWrite(void* cpu,longptr rip,longptr stkaddr,int size)
{
	int isSameBlock = updateAndCheckLastBlock(cpu);
	int loged = 0;
	if (gnmid > 0)
	{	//如果成员ID栈里面有成员，那么我们当成成员迁移来处理
		if (isSameBlock)
		{	//如果还在同一个OP分支里面，我们才处理
			gnmid--;
			int mid = gmidstack[gnmid];
			mid &= ~mid_state_dirty;
			xst_set(gvstkst,stkaddr,size,mid);
			addrprintf(rip,"[%d]:var%d -> addr:%08X->size:%d",glastBlockID,mid,stkaddr,size);
			loged = 1;
		}
	}
	if (loged == 0)
		addrprintf(rip,"[%d]:VStack Write->addr:%08X->size:%d",glastBlockID,stkaddr,size);
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
				accessContextRead(cpu,(longptr)rip,ctxoftr,szrmem);
		}
		//这里我们监控对虚拟栈的访问
		valr = vmexec_getReg(cpu,VM_REG_RBP);
		if (lprmem >= valr)
		{	//如果访问发生在Context内
			longptr ctxoftr = lprmem - valr;
			//计算偏移，如果偏移小于0xA0我们认为是有效的Context访问
			if (ctxoftr < 0x20)
				accessVStackRead(cpu,(longptr)rip,lprmem,szrmem);
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
				accessContextWrite(cpu,(longptr)rip,ctxoftw,szwmem);
		}
		//这里我们监控对虚拟栈的访问
		valw = vmexec_getReg(cpu,VM_REG_RBP);
		if (lpwmem >= valw)
		{	//如果访问发生在Context内
			longptr ctxoftw = lpwmem - valw;
			//计算偏移，如果偏移小于0xA0我们认为是有效的Context访问
			if (ctxoftw < 0x20)
				accessVStackWrite(cpu,(longptr)rip,lpwmem,szwmem);
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
	//申请一个状态实例来跟踪Context
	gxstate = xst_allocstate();
	//申请一个状态实例来跟踪Stack
	gvstkst = xst_allocstate();
	//这里我们返回全局脚本标记，这样脚本只会在第一次编译
	return main_ret_global;
}