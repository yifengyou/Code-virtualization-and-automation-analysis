// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 XREMOTEIDA_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// XREMOTEIDA_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef XREMOTEIDA_EXPORTS
#define XREMOTEIDA_API __declspec(dllexport)
#else
#define XREMOTEIDA_API __declspec(dllimport)
#endif

#define REMOTEIDA_PIPE_TEMP			_T("\\\\%s\\Pipe\\XREMOTEIDA_%d")
#define REMOTEIDA_PIPE_NOTICE		_T("\\\\%s\\pipe\\xIDANotice%04X")
#define MAX_PIPE_PACKET	0x10000
#define MAX_IDA_INST	4


enum xida_vids{
	xida_vid_syncip = 0
};

enum xida_vid_nodes{
	xida_vid_address = 0,
	xida_vid_pipeid
};


enum xida_msg
{
	xida_msgid_getinfo,
	xida_msgid_info,
	xida_msgid_getsymbol,
	xida_msgid_symbol,
	xida_msgid_findsym,
	xida_msgid_setip,
	xida_msgid_getcmt,
	xida_msgid_cmt
};
struct xida_msg_hdr { 
	xida_msg	msgid;
};


struct xida_msg_getinfo : public xida_msg_hdr 
{
	struct  xida_msg_getinfo() { msgid = xida_msgid_getinfo; }
	DWORD	pid;	//ollydbg2's pid
	char	computerName[MAX_COMPUTERNAME_LENGTH+1];
};

struct xida_msg_info : public xida_msg_hdr
{
	struct  xida_msg_info() { msgid = xida_msgid_info; }
	char	modname[MAX_PATH];
	char	peHdr[0x1000];
};
#define sym_normal		0x1
#define sym_locallab	0x2
#define sym_stack		0x4

struct xida_msg_syminfo : public xida_msg_hdr
{
	struct  xida_msg_syminfo() { msgid = xida_msgid_getsymbol; 
	flags = sym_normal | sym_locallab;
	}
	long	rva;
	long	flags;
};

struct xida_msg_setip : public xida_msg_hdr
{
	struct  xida_msg_setip() { msgid = xida_msgid_setip; }
	long	rva;
};

struct xida_msg_getcmt : public xida_msg_hdr
{
	struct  xida_msg_getcmt() { msgid = xida_msgid_getcmt; }
	long	rva;
};
struct xida_msg_findsym : public xida_msg_hdr
{
	struct  xida_msg_findsym() { msgid = xida_msgid_findsym; }
	char	symname[256];
};
struct xida_msg_symbol : public xida_msg_hdr
{
	struct  xida_msg_symbol() { msgid = xida_msgid_symbol; }
	char	symname[256];
};

struct xida_msg_comment : public xida_msg_hdr
{
	struct  xida_msg_comment() { msgid = xida_msgid_cmt; }
	char	comment[256];
};

extern int gPipeId;