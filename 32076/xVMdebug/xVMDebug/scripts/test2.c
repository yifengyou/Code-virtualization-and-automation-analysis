#include <windows.h>
//利用虚拟机进行高级自动追踪分析例子
int main(int argc,char** argv)
{
	//设置最大的追踪记录，
	//虚拟机运行将只保留设定数量的记录
	//如果设定为0那么不保留追踪记录
	//LoadLibraryA("advapi32.dll");
	//DebugBreak();
	//vcall(LoadLibraryA,1,"advapi32.dll");
	xvm_setBackTrace(5000);
	
	return 0;
}