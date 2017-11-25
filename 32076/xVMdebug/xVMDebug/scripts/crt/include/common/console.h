#ifndef Console_h__
#define Console_h__
#include <Windows.h>
#include <tchar.h>
#include <WinCon.h>

TCHAR* trim_co(TCHAR * pStr);
typedef int (__stdcall* PENTRY_CmdCallBack)(TCHAR** pArgV,long pArgC);

typedef struct _s_congui{
	HANDLE m_std_input;
	HANDLE m_std_output;
	HANDLE m_std_error;
	HWND	m_hconsole;
	TCHAR*	m_lpInBuf;
	TCHAR*	m_argv[64];
	BOOL	m_Paraed;
	HANDLE m_hCmdTD;
	PENTRY_CmdCallBack m_CallBack;
}s_congui,*sp_congui;

void congui_init(s_congui* pgui);

void ClearScreen(s_congui* pgui);
int  CreateConsole(s_congui* pgui,TCHAR* pTitle);
int	 CloseConsole(s_congui* pgui);
void SetColor(s_congui* pgui,unsigned short ForeColor,unsigned short BackGroundColor);
BOOL ShowConsole(s_congui* pgui);
BOOL HideConsole(s_congui* pgui);
void MoveText(s_congui* pgui,int x, int y, SMALL_RECT rc);
void DeleteLine(s_congui* pgui,int row);
void ControlStatus(s_congui* pgui,DWORD state);
void CharWindow(s_congui* pgui,TCHAR ch, SMALL_RECT rc);
void DeleteTopLine(s_congui* pgui,SMALL_RECT rc);
BOOL Printf(s_congui* pgui,TCHAR* pFormat,... );
void PrintMem(s_congui* pgui,BYTE* plpMem,int pszMem,TCHAR* plpFormat,int pnlChar);
void AllocInputBuf(s_congui* pgui,int pszBuf);
void FreeInputBuf(s_congui* pgui);
void PutInput(s_congui* pgui,TCHAR* pLine);
TCHAR* GetInput(s_congui* pgui);
TCHAR** GetArgv(s_congui* pgui);
int	 GetArgc(s_congui* pgui);
int	 FindArgv(s_congui* pgui,TCHAR* pCmd);
void DeleteArgv(s_congui* pgui,int pIndex);
int	 FindAndDel(s_congui* pgui,TCHAR* pCmd);
void SetOutputHandle(s_congui* pgui,HANDLE phOut);
HANDLE CreateCmdThread(s_congui* pgui,PENTRY_CmdCallBack pCmdProc);
BOOL TerminateCmdThread(s_congui* pgui);
void ShadowWindowLine(s_congui* pgui,TCHAR *str);
void DrawBox(s_congui* pgui,BOOL bSingle, SMALL_RECT rc);
int	 WriteInput(s_congui* pgui,TCHAR* psText);

//SetConsoleFont(HANDLE pstdout,
#endif // Console_h__

