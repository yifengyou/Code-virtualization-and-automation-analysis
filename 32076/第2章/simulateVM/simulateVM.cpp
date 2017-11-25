// simulateVM.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include "Q:/Projects/nSafeSuite/ncvm/nxfmt/nxfmt86.h"
#include "../VMCrt/VMCrt.h"
/************************************************************************/
/* 虚拟化媒介编码器+译码器                                              */
/************************************************************************/
// Parameter: long rva	指定代码的rva，实际上我们这里没有使用
// Parameter: const unsigned char * lpCode	指定虚拟化目标代码内存位置
// Parameter: int szCode	虚拟化目标代码大小
// Parameter: char * lpOpcode	编码后的虚拟化媒介(opcode)内存地址
//************************************
int encodeOpcode(long rva,const unsigned char* lpCode,int szCode,char* lpOpcode)
{	
	int oppos = 0;
	const unsigned char* lpend = lpCode + szCode;
	while (lpCode < lpend)
	{
		//对被虚拟化指令进行译码，由于是样机，因此我们没有使用反汇编引擎而是直接硬编
		switch (*lpCode)	
		{
		case 0xB8://mov eax,imm	//这是因为我们已知被虚拟化指令，因此硬编
		{
			lpOpcode[oppos++] = op_mov;	//我们生成opcode为mov的动作
			lpOpcode[oppos++] = VREG_RAX;	//将原始指令当中的参数编码到opcode当中
			*(long*)(lpOpcode + oppos) = *(long*)(lpCode+1);	//将常量编码进去
			oppos += sizeof(long);
			lpCode += 5;
		}break;
		case 0x05://add eax,imm
		{
			lpOpcode[oppos++] = op_add;	//编码为add
			lpOpcode[oppos++] = VREG_RAX;	//编码参数
			*(long*)(lpOpcode + oppos) = *(long*)(lpCode+1);	//编码常量
			oppos += sizeof(long);
			lpCode += 5;
		}break;
		}
	}
	lpOpcode[oppos++] = op_exit;	//最后我们编码一个退出动作来退出虚拟机
	*(long*)(lpOpcode + oppos) = 0x401010;	//指定退出地址
	oppos += sizeof(long);
	return oppos;
}

int _tmain(int argc, _TCHAR* argv[])
{
	nxfmt::pe xpesrc; //分别定义三个PE文件管理类，实际上负责从PE中读取或者写入数据
	nxfmt::pe xpedst;
	nxfmt::pe xpevmcrt;
		//虚拟化目标程式
	if (xpesrc.open(_T("Q:\\Documents\\Books\\third\\code\\模拟虚拟化\\vm1.exe"),nxfmt::open_readonly) < 0)
		return -1;
	//我们的运行时代码的模块文件
	if (xpevmcrt.open(_T("Q:\\Documents\\Books\\third\\code\\模拟虚拟化\\simulateVM\\Release\\VMCrt.dll"),nxfmt::open_readonly) < 0)
		return -2;
	//生成的新程式文件
	if (xpedst.open(_T("Q:\\Documents\\Books\\third\\code\\模拟虚拟化\\vm1_out.exe"),nxfmt::open_readwrite) < 0)
		return -3;
	//这里先取得我们虚拟化目标代码的内存地址，0x1006是我们事先已知的被虚拟化指令的RVA地址
	const unsigned char* lptc = (const unsigned char*)xpesrc.data(0x1006);
	//定义一个空间在缓存虚拟化媒介
	char opbuf[0x100];
	//进行译码和编码
	int oplen = encodeOpcode(0x1006,lptc,10,opbuf);
	//根据原始提供的程式复制出一个只包含所有PE数据的文件，也就是附加数据被剔除
	xpedst.copyFromPE(xpesrc.data(),xpesrc.size());
	//在复制出来的文件当中添加一个区段到末尾，并且将区段设定为可执行，0x10000大小
	long secrva = xpedst.add_section(0x10000,0,0x10000,0xE0000020,".vm");
	if (secrva == 0)
		return -4;
	//取得新添加区段的内存地址，这样我们就可以写入数据了
	char* lpsec = xpedst.data(secrva);
	//首先我们先将opcode写入刚添加的区段
	memcpy(lpsec,opbuf,oplen);
	//接着我们读取出运行时代码的数据
	const char* lpvmcrt = xpevmcrt.data(0x1000);
	long vmcrtentry = xpevmcrt.entry();
	//我们这里假设运行时代码不超过1000字节大小
	char crtbuf[1000];
	memcpy(crtbuf,lpvmcrt,sizeof(crtbuf));
	//这里我们要根据运行时代码的入口就算出相对偏移，然后修正我们在运行时代码设计时保留的opcode地址
	long oftentry = vmcrtentry + 1 - 0x1000;
	//这里我们要将opcode地址修正到运行时代码当中去
	*(long*)(crtbuf + oftentry) = secrva+ 0x400000;
	//将运行时代码写入目标文件区段
	memcpy(lpsec + oplen,crtbuf,sizeof(crtbuf));
	//计算在区段中运行时代码的入口
	long addrentry = vmcrtentry - 0x1000 + oplen + secrva;
	//这里我们定位到被虚拟化代码
	char* lpOrg = xpedst.data(0x1006);
	//由于代码已经被虚拟化，因此我们可以将原始的代码清除掉
	memset(lpOrg,0x90,10);
	//我们在原始代码执行处添加一个条件转移指令
	*lpOrg++ = 0xE9;
	*(long*)lpOrg = addrentry - 0x1006 - 5;
	//打完收工
	xpedst.close();
	return 0;
}

