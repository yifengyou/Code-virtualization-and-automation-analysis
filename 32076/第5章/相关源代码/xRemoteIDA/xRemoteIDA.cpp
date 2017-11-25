// xRemoteIDA.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "xRemoteIDA.h"
#include "XIDAServer.h"

extern plugin_t PLUGIN;

XIDAServer gIDAServer;
int gPipeId = -1; 

void initPipeServer()
{

	gIDAServer.nnStartup(1);
	TCHAR buf[256];
	for (int i = 0; i < MAX_IDA_INST; i++)
	{
		wsprintfW(buf,REMOTEIDA_PIPE_TEMP,L".",i);
		HANDLE hTestPipe = CreateFile(buf,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
		if (hTestPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hTestPipe);
			continue;
		}
		if (gIDAServer.ListenOnPipe(buf) > 0)
		{
			gPipeId = i;
			break;
		}
			
	}
}
bool idaapi menu_item_syncip(void *ud)
{
	ea_t addr = get_screen_ea();
	NPacketBuffer pk(xida_vid_syncip);
	xida_msg_setip setip;
	pk.putInt(xida_vid_address,addr - get_imagebase());
	pk.putInt(xida_vid_pipeid,gPipeId);
	gIDAServer.broadcast(&pk);
	return false;
}
int idaapi init(void)
{
	static BOOL isAdded = FALSE;
	if (!isAdded)
	{
		isAdded = TRUE;
		bool badd = add_menu_item("View","Sync IP to Ollydbg2","Ctrl-Alt-S",0,menu_item_syncip,0);
		int p = 0;
		p++;
	}

	return PLUGIN_KEEP;
}

//--------------------------------------------------------------------------
//      Terminate.
//      Usually this callback is empty.
//      The plugin should unhook from the notification lists if
//      hook_to_notification_point() was used.
//
//      IDA will call this function before unloading the plugin.

void idaapi term(void)
{

}

//--------------------------------------------------------------------------
//
//      The plugin method
//
//      This is the main function of plugin.
//
//      It will be called when the user selects the plugin.
//
//              arg - the input argument, it can be specified in
//                    plugins.cfg file. The default is zero.
//
//

void idaapi run(int arg)
{
	MessageBox(GetForegroundWindow(),_T("本插件为\r\n\t《软件保护与分析技术》 \r\n\t《代码虚拟与自动化分析》\r\n随书演示程式.\n请勿用于任何除配合理解书中内容的任何其它用途。"),_T("About xVMDebug"),MB_OK | MB_ICONINFORMATION);
}

char comment[] = "This is a part of xVMDebug plugin. It's made ollydbg2 connected to IDA.";

char help[] =
"xRemoteIDA\n"
"\n"
"This module made ollydbg2 work with IDA.\n"
"\n"
"A Part of xVMDebug\n";

//--------------------------------------------------------------------------
// This is the preferred name of the plugin module in the menu system
// The preferred name may be overriden in plugins.cfg file

char wanted_name[] = "xRemoteIDA plugin";


// This is the preferred hotkey for the plugin module
// The preferred hotkey may be overriden in plugins.cfg file
// Note: IDA won't tell you if the hotkey is not correct
//       It will just disable the hotkey.

char wanted_hotkey[] = "Ctrl-Alt-X";


plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	PLUGIN_UNL,           // plugin flags
	init,                 // initialize

	term,                 // terminate. this pointer may be NULL.

	run,                  // invoke plugin

	comment,              // long comment about the plugin
	// it could appear in the status line
	// or as a hint

	help,                 // multiline help about the plugin

	wanted_name,          // the preferred short name of the plugin
	wanted_hotkey         // the preferred hotkey to run the plugin
};
