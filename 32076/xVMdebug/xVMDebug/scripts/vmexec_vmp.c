#include <windows.h>
#include "internal.h"
//利用虚拟机进行高级自动追踪分析例子

//单条指令执行前回调
int		vmexec_before_step(void* cpu,void* rip,void* inst)
{	//我们简单的判断是否ripv在系统代码内，如果是我们就暂停追踪
	unsigned long ripv = (unsigned long)rip;
	if (ripv >= 0x60000000 && ripv <= 0x7F000000)
		return vmexec_ret_finished;
	return vmexec_ret_normal;
}
//单条指令执行后回调
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}