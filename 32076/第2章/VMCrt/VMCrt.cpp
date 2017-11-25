// VMCrt.cpp : 定义 DLL 应用程序的导出函数。
//

#include "VMCrt.h"
#pragma comment(linker,"/ENTRY:simVMEntry")


struct x86Ctx	//现场结构，根据push 0x12345678,pushfd,pushad确定
{
	long edi;
	long esi;
	long ebp;
	long esp;
	long ebx;
	long edx;
	long ecx;
	long eax;
	long eflags;
	union{	//在入口时，push 0x12345678当做OP指针，出口时当做返回地址
		long	opcode;
		void*	retaddr;
	};

};


struct vcpu_ctx
{
	long regs[8];	//寄存器
	long eflags;	//用于存放标志寄存器
	const char* lpopcode;	//存放opcode指针
};


void* simVMLoop(vcpu_ctx* ctx)
{
	const char* lpop = ctx->lpopcode;	//这里指向了我们的虚拟化媒介
	while (1)
	{
		switch (*lpop++)	//解码并执行
		{
		case op_mov:{//模拟执行指令，
			int regid = *lpop++;	//读取虚拟媒介当中的寄存器编号
			ctx->regs[regid] = *(long*)lpop;	//取出常量并设置到对应的虚拟寄存器当中去
			lpop += sizeof(long);
		}break;
		case op_add:{
			int regid = *lpop++;	//读取虚拟媒介当中的寄存器编号
			//取出常量并模拟执行add相加算法
			ctx->regs[regid] += *(long*)lpop;
			lpop += sizeof(long);
		}break;
		case op_exit:
		{ //遇到出口我们直接设定出口地址，并退出解码执行循环
			return *(void**)lpop;
		}break;
		}
	}
	return 0;
}


void __stdcall simVMInit(x86Ctx* x86ctx)
{
	vcpu_ctx vctx;	//直接初始化一个虚拟化环境
	vctx.regs[VREG_RAX] = x86ctx->eax;	//迁移数据到虚拟化环境
	vctx.regs[VREG_RCX] = x86ctx->ecx;
	vctx.regs[VREG_RDX] = x86ctx->edx;
	vctx.regs[VREG_RBX] = x86ctx->ebx;
	vctx.regs[VREG_RSP] = x86ctx->esp;
	vctx.regs[VREG_RBP] = x86ctx->ebp;
	vctx.regs[VREG_RSI] = x86ctx->esi;
	vctx.regs[VREG_RDI] = x86ctx->edi;
	vctx.lpopcode = (const char*)x86ctx->opcode;
	vctx.eflags = x86ctx->eflags;
	void* lpout = simVMLoop(&vctx);	//进入解码执行
	x86ctx->eax = vctx.regs[VREG_RAX];	//迁出数据到实际环境
	x86ctx->ecx = vctx.regs[VREG_RCX];
	x86ctx->edx = vctx.regs[VREG_RDX];
	x86ctx->ebx = vctx.regs[VREG_RBX];
	x86ctx->esp = vctx.regs[VREG_RSP];
	x86ctx->ebp = vctx.regs[VREG_RBP];
	x86ctx->esi = vctx.regs[VREG_RSI];
	x86ctx->edi = vctx.regs[VREG_RDI];
	x86ctx->eflags = vctx.eflags;
	x86ctx->retaddr = lpout;	//设置出口地址
}


void __declspec(naked) simVMEntry()
{
	__asm{
		//这里我们设计一个站位指令，实际指令的常量在打包代码到目标程式中时会被替换成OPCODE指针
		push 0x12345678		
		pushfd	//保存eflags
		pushad	//保存通用寄存器，由于我们的运行时部件不影响其它寄存器因此不用保存
		push esp	//为了方便我们直接将上面压入的数据转换成一个结构，push esp就是结构指针
		call simVMInit	//调用初始化函数
		popad		//出口代码，恢复通用寄存器
		popfd		//恢复标志寄存器
		ret			//这里我们将OPCODE指针占用的空间，在出口上用作返回地址
	}
}


