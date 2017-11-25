#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "console.h"


void trimLeft_co(TCHAR * pChar){ 
	TCHAR *pBuff = pChar; 
	while( *pBuff == ' ' || *pBuff == '\t' || *pBuff == '\r' || *pBuff == '\n'){ 
		++ pBuff; 
	}
	memmove(pChar,pBuff,_tcslen(pBuff)+1);
}
void trimRight_co(TCHAR* pChar){
	int pLen;
	int i;
	pLen = _tcslen(pChar);
	for(i = pLen - 1; i >= 0; i-- ){
		if( *(pChar + i) == ' ' || *(pChar + i) == '\t' || *(pChar + i) == '\r' || *(pChar + i) == '\n') {
			*(pChar + i ) = '\0';
		}else break;
	}
}

TCHAR* trim_co(TCHAR * pStr){

	if (!pStr)
		return _T("");
	trimLeft_co(pStr);
	trimRight_co(pStr);
	return pStr;
}

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case	CTRL_CLOSE_EVENT:
		FreeConsole();
		return TRUE;
	}
	return FALSE;
}
void congui_init(s_congui* pgui)
{
	pgui->m_std_output = INVALID_HANDLE_VALUE;
	pgui->m_std_input = INVALID_HANDLE_VALUE;
	pgui->m_std_error = INVALID_HANDLE_VALUE;
	pgui->m_lpInBuf = NULL;
	pgui->m_Paraed = TRUE;
	pgui->m_CallBack = NULL;
}

void ClearScreen( s_congui* pgui )
{

	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo(pgui->m_std_output, &bInfo ); 
	COORD home = {0, 0};
	WORD att = bInfo.wAttributes;
	DWORD pwtd = 0;
	unsigned long size = bInfo.dwSize.X * bInfo.dwSize.Y;
	FillConsoleOutputAttribute(pgui->m_std_output, att, size, home, &pwtd);
	FillConsoleOutputCharacter(pgui->m_std_output, ' ', size, home, &pwtd);
	COORD pos = {0,0};
	SetConsoleCursorPosition(pgui->m_std_output, pos); // 设置光标位置

}



int CreateConsole(s_congui* pgui,TCHAR* pTitle)
{
	AllocConsole();
// 	if (!AllocConsole())
// 		return FALSE;
	pgui->m_std_input = GetStdHandle(STD_INPUT_HANDLE);
	pgui->m_std_output = GetStdHandle(STD_OUTPUT_HANDLE);
	pgui->m_std_error = GetStdHandle(STD_ERROR_HANDLE);
	pgui->m_hconsole = (HWND)GetConsoleWindow();
	SetConsoleTitle(pTitle);
	int hCrt = _open_osfhandle((long)pgui->m_std_output,_O_TEXT );  
	FILE* hf = _fdopen( hCrt, "w" );
	*stdout = *hf;
	setvbuf( stdout, NULL, _IONBF, 0 );  
	SetColor(pgui,7,0);
	return TRUE;
}

void SetColor(s_congui* pgui,unsigned short ForeColor,unsigned short BackGroundColor)
{
	SetConsoleTextAttribute(pgui->m_std_output,ForeColor|BackGroundColor);
}

void CharWindow(s_congui* pgui,TCHAR ch, SMALL_RECT rc) // 将ch输入到指定的窗口中
{
	static COORD chPos;
	chPos.X =rc.Left+1;
	chPos.Y =rc.Top+1;
	SetConsoleCursorPosition(pgui->m_std_output, chPos); // 设置光标位置
	if ((ch<0x20)||(ch>0x7e)) return;
	WriteConsoleOutputCharacter(pgui->m_std_output, &ch, 1, chPos, NULL);
	if (chPos.X>=(rc.Right-1))
	{
		chPos.X = rc.Left;
		chPos.Y++;
	}
	if (chPos.Y>(rc.Bottom-1)) 
	{
		DeleteTopLine(pgui,rc);
		chPos.Y = rc.Bottom-1;
	}
	chPos.X++;
	SetConsoleCursorPosition(pgui->m_std_output, chPos); // 设置光标位置
}


void DeleteTopLine(s_congui* pgui,SMALL_RECT rc)
{
	COORD crDest;
	CHAR_INFO chFill;
	SMALL_RECT rcClip = rc;
	rcClip.Left++; rcClip.Right--;
	rcClip.Top++; rcClip.Bottom--;
	crDest.X = rcClip.Left;
	crDest.Y = rcClip.Top - 1;
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo( pgui->m_std_output, &bInfo ); 
	chFill.Attributes = bInfo.wAttributes;
	chFill.Char.AsciiChar = ' ';
	ScrollConsoleScreenBuffer(pgui->m_std_output, &rcClip, &rcClip, crDest, &chFill);
}
void ControlStatus(s_congui* pgui,DWORD state) // 在最后一行显示控制键的状态
{
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo( pgui->m_std_output, &bInfo ); 
	COORD home = {0,0};
	WORD att0 = BACKGROUND_INTENSITY ;
	WORD att1 = FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_RED;
	FillConsoleOutputAttribute(pgui->m_std_output, att0, bInfo.dwSize.X, home, NULL);
	FillConsoleOutputCharacter(pgui->m_std_output, ' ', bInfo.dwSize.X, home, NULL);
	SetConsoleTextAttribute(pgui->m_std_output, att1);
	COORD staPos = {bInfo.dwSize.X-16,bInfo.dwSize.Y-1};
	SetConsoleCursorPosition(pgui->m_std_output, staPos);
	if (state & NUMLOCK_ON) 
		WriteConsole(pgui->m_std_output, "NUM", 3, NULL, NULL);
	staPos.X += 4;
	SetConsoleCursorPosition(pgui->m_std_output, staPos);
	if (state & CAPSLOCK_ON) 
		WriteConsole(pgui->m_std_output, "CAPS", 4, NULL, NULL);
	staPos.X += 5;
	SetConsoleCursorPosition(pgui->m_std_output, staPos);
	if (state & SCROLLLOCK_ON) 
		WriteConsole(pgui->m_std_output, "SCROLL", 6, NULL, NULL);
	SetConsoleTextAttribute(pgui->m_std_output, bInfo.wAttributes); // 恢复原来的属性
	SetConsoleCursorPosition(pgui->m_std_output, bInfo.dwCursorPosition); // 恢复原来的光标位置
}


void MoveText(s_congui* pgui,int x, int y, SMALL_RECT rc)
{
	COORD crDest = {x, y};
	CHAR_INFO chFill;
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo( pgui->m_std_output, &bInfo ); 
	chFill.Attributes = bInfo.wAttributes;
	chFill.Char.AsciiChar = ' ';
	ScrollConsoleScreenBuffer(pgui->m_std_output, &rc, NULL, crDest, &chFill);
}
void DeleteLine(s_congui* pgui,int row)
{
	SMALL_RECT rcScroll, rcClip;
	COORD crDest = {0, row - 1};
	CHAR_INFO chFill;
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo( pgui->m_std_output, &bInfo ); 
	rcScroll.Left = 0;
	rcScroll.Top = row;
	rcScroll.Right = bInfo.dwSize.X - 1;
	rcScroll.Bottom = bInfo.dwSize.Y - 1;
	rcClip = rcScroll;
	chFill.Attributes = bInfo.wAttributes;
	chFill.Char.AsciiChar = ' ';
	ScrollConsoleScreenBuffer(pgui->m_std_output, &rcScroll, &rcClip, crDest, &chFill);
}


BOOL Printf(s_congui* pgui,TCHAR* pFormat,... )
{
	BOOL pRet = TRUE;
	if (pgui->m_std_output != INVALID_HANDLE_VALUE)
	{
// 		CONSOLE_SCREEN_BUFFER_INFO p_info;
// 		GetConsoleScreenBufferInfo(pgui->m_std_output,&p_info);
// 		p_info.dwCursorPosition.X = 0;
// 		SetConsoleCursorPosition(pgui->m_std_output,p_info.dwCursorPosition);
		TCHAR pDstBuf[2048];
		va_list m_ap; 
		va_start(m_ap, pFormat); 
		int m_n = wvsprintf(pDstBuf,pFormat,m_ap);
		va_end(m_ap);
		DWORD pwtd = 0;
		pRet = WriteConsole(pgui->m_std_output,pDstBuf,m_n,&pwtd,NULL);
	}
	return pRet;
}

int CloseConsole(s_congui* pgui)
{
	pgui->m_std_output = INVALID_HANDLE_VALUE;
	pgui->m_std_input = INVALID_HANDLE_VALUE;
	pgui->m_std_error = INVALID_HANDLE_VALUE;
	SetConsoleCtrlHandler(ConsoleHandlerRoutine,FALSE);
	FreeConsole();
	return ERROR_SUCCESS;
}

BOOL ShowConsole(s_congui* pgui)
{
	return ShowWindow(pgui->m_hconsole,SW_SHOW);
}

BOOL HideConsole(s_congui* pgui)
{
	return ShowWindow(pgui->m_hconsole,SW_HIDE);
}

TCHAR* GetInput(s_congui* pgui)
{

	if (!pgui->m_lpInBuf)
	{
		AllocInputBuf(pgui,16*1024);
	}
	DWORD pReaded = 0;
	if (ReadConsole(pgui->m_std_input,pgui->m_lpInBuf,16*1024,&pReaded,NULL))
	{
		pgui->m_Paraed = FALSE;
		pgui->m_lpInBuf[pReaded] = 0;
		trim_co(pgui->m_lpInBuf);
		return pgui->m_lpInBuf;
	}
	return NULL;
}

void SetOutputHandle(s_congui* pgui,HANDLE phOut )
{
	pgui->m_std_output = phOut;
}

TCHAR** GetArgv(s_congui* pgui)
{
	int i;
	if (pgui->m_Paraed)
		return pgui->m_argv;
	pgui->m_Paraed = TRUE;
	pgui->m_argv[0] = NULL;
	trim_co(pgui->m_lpInBuf);
	int psLen = _tcslen(pgui->m_lpInBuf);
	if (psLen == 0)
		return pgui->m_argv;
	int x=0;
	pgui->m_argv[0] = pgui->m_lpInBuf;
	x++;
	BOOL pDotOn = FALSE;
	for (i=0;i<psLen;i++)
	{
		if (pgui->m_lpInBuf[i] == ' ' && !pDotOn)
		{
			pgui->m_lpInBuf[i] = 0;
			i++;
			while (pgui->m_lpInBuf[i] == ' ' || pgui->m_lpInBuf[i] == '\t' || pgui->m_lpInBuf[i] == '\r' || pgui->m_lpInBuf[i] == '\n')
				i++;
			pgui->m_argv[x] = pgui->m_lpInBuf+i;
			x++;
			if (x >= 63)
				break;
		}else if (pgui->m_lpInBuf[i] == '"')
			pDotOn = !pDotOn;
	}
	pgui->m_argv[x] = NULL;
	return pgui->m_argv;
}

int GetArgc(s_congui* pgui)
{
	int i;
	for (i=0;i<64;i++)
	{
		if (pgui->m_argv[i] == NULL)
			return i;
	}
	return 0;
}

void PrintMem(s_congui* pgui, BYTE* plpMem,int pszMem,TCHAR* plpFormat,int pnlChar /*= 0*/ )
{
	if (pszMem <= 0)
		return;
	int i;
	if (pnlChar == 0)
	{
		for (i=0;i<pszMem;i++)
		{
			if (i==pszMem-1)
				Printf(pgui,_T("%02X"),(TCHAR*)plpMem[i]);
			else
				Printf(pgui,_T("%02X%s"),(TCHAR*)plpMem[i],plpFormat);
		}
	}else
	{
		for (i=0;i<pszMem;i++)
		{
			if (i>=1 && i%pnlChar == 0)
				Printf(pgui,_T("\n"));
			if (i==pszMem-1)
				Printf(pgui,_T("%02X"),(TCHAR*)plpMem[i]);
			else
				Printf(pgui,_T("%02X%s"),(TCHAR*)plpMem[i],plpFormat);
		}
	}
	Printf(pgui,_T("\n"));
}

void AllocInputBuf( s_congui* pgui,int pszBuf )
{
	if (pgui->m_lpInBuf)
		free(pgui->m_lpInBuf);
	pgui->m_lpInBuf = (TCHAR*)malloc(pszBuf);
	memset(pgui->m_lpInBuf,0,pszBuf);
}

void PutInput(s_congui* pgui, TCHAR* pLine )
{
	if (!pgui->m_lpInBuf)
		AllocInputBuf(pgui,_tcslen(pLine+1));
	_tcscpy(pgui->m_lpInBuf,pLine);
	pgui->m_Paraed = FALSE;
}

void FreeInputBuf(s_congui* pgui)
{
	if (pgui->m_lpInBuf)
		free(pgui->m_lpInBuf);
	pgui->m_lpInBuf = NULL;
}

int FindArgv( s_congui* pgui,TCHAR* pCmd )
{
	int pc = GetArgc(pgui);
	int i;
	for (i=0;i<pc;i++)
	{
		if (_tcsicmp(pCmd,GetArgv(pgui)[i]) == 0)
			return i;
	}
	return -1;
}

void DeleteArgv( s_congui* pgui,int pIndex )
{
	TCHAR** pArgV = GetArgv(pgui);
	if (pIndex < GetArgc(pgui))
	{
		pArgV+=pIndex;
		while (*pArgV)
		{
			*pArgV=*(pArgV+1);
			pArgV++;
		}
	}
}

int FindAndDel(s_congui* pgui, TCHAR* pCmd )
{
	int pidx = FindArgv(pgui,pCmd);
	if (pidx != -1)
	{
		DeleteArgv(pgui,pidx);
	}
	return pidx;
}
long __stdcall CmdThreadCore(s_congui* pgui)
{
	while(GetInput(pgui))
	{
		if (pgui->m_CallBack)
		{
			int pRetCode = pgui->m_CallBack(GetArgv(pgui),GetArgc(pgui));
			if (pRetCode < 0)
				break;
		}
	}
	return 0;
}
HANDLE CreateCmdThread(s_congui* pgui,PENTRY_CmdCallBack pCmdProc)
{
	if (!pCmdProc)
		return NULL;
	pgui->m_CallBack = pCmdProc;
	return CreateThread((LPSECURITY_ATTRIBUTES)NULL,0,(LPTHREAD_START_ROUTINE)CmdThreadCore,(LPVOID)pgui,(DWORD)NULL,(LPDWORD)NULL);
}

BOOL TerminateCmdThread(s_congui* pgui)
{
	return TerminateThread(pgui->m_hCmdTD,0);
}

void ShadowWindowLine(s_congui* pgui,TCHAR *str)
{
	CONSOLE_SCREEN_BUFFER_INFO bInfo; // 窗口缓冲区信息
	GetConsoleScreenBufferInfo( pgui->m_std_output, &bInfo ); // 获取窗口缓冲区信息
	// 计算显示窗口大小和位置
	int x1, y1, x2, y2, chNum = _tcslen(str);
	x1 = (bInfo.dwSize.X - chNum)/2 - 2;
	y1 = bInfo.dwSize.Y/2 - 2;
	x2 = x1 + chNum + 4;
	y2 = y1 + 5;
	WORD att1 = BACKGROUND_INTENSITY; // 阴影属性
	WORD att0 = FOREGROUND_RED |FOREGROUND_GREEN |FOREGROUND_BLUE | FOREGROUND_INTENSITY |	BACKGROUND_RED | BACKGROUND_BLUE; // 文本属性
	WORD attText = FOREGROUND_RED |FOREGROUND_INTENSITY; // 文本属性
	// 设置阴影
	COORD posShadow = {x1+1, y1+1}, posText = {x1, y1};
	int i;
	for (i=0; i<5; i++){
		FillConsoleOutputAttribute(pgui->m_std_output, att1, chNum + 4, posShadow, NULL); 
		posShadow.Y++;
	}
	// 填充窗口背景
	for (i=0; i<5; i++){
		FillConsoleOutputAttribute(pgui->m_std_output, att0, chNum + 4, posText, NULL); 
		posText.Y++;
	}
	// 写文本和边框
	posText.X = x1 + 2;
	posText.Y = y1 + 2;
	WriteConsoleOutputCharacter(pgui->m_std_output, str, _tcslen(str), posText, NULL);
	SMALL_RECT rc = {x1, y1, x2-1, y2-1};
	DrawBox(pgui,TRUE, rc);
	SetConsoleTextAttribute(pgui->m_std_output, bInfo.wAttributes); // 恢复原来的属性
}
void DrawBox(s_congui* pgui,BOOL bSingle, SMALL_RECT rc)
{
	TCHAR chBox[6];
	if (bSingle) {
		chBox[0] = (char)0xda; // 左上角点
		chBox[1] = (char)0xbf; // 右上角点
		chBox[2] = (char)0xc0; // 左下角点
		chBox[3] = (char)0xd9; // 右下角点
		chBox[4] = (char)0xc4; // 水平
		chBox[5] = (char)0xb3; // 坚直
	} else {
		chBox[0] = (char)0xc9; // 左上角点
		chBox[1] = (char)0xbb; // 右上角点
		chBox[2] = (char)0xc8; // 左下角点
		chBox[3] = (char)0xbc; // 右下角点
		chBox[4] = (char)0xcd; // 水平
		chBox[5] = (char)0xba; // 坚直
	}
	COORD pos = {rc.Left, rc.Top};
	WriteConsoleOutputCharacter(pgui->m_std_output, &chBox[0], 1, pos, NULL);
	for (pos.X = rc.Left + 1; pos.X < rc.Right-1;pos.X++)
		WriteConsoleOutputCharacter(pgui->m_std_output, &chBox[4], 1, pos, NULL);
		pos.X = rc.Right;
	WriteConsoleOutputCharacter(pgui->m_std_output, &chBox[1], 1, pos, NULL);
	for (pos.Y = rc.Top+1; pos.Y<rc.Bottom-1;pos.Y++)
	{
		pos.X = rc.Left;
		WriteConsoleOutputCharacter(pgui->m_std_output, &chBox[5], 1, pos, NULL);
		pos.X = rc.Right;
		WriteConsoleOutputCharacter(pgui->m_std_output, &chBox[5], 1, pos, NULL);
	}
	pos.X = rc.Left; pos.Y = rc.Bottom;
	WriteConsoleOutputCharacter(pgui->m_std_output, &chBox[2], 1, pos, NULL);
	for (pos.X = rc.Left + 1; pos.X<rc.Right-1;pos.X++)
		WriteConsoleOutputCharacter(pgui->m_std_output, &chBox[4], 1, pos, NULL);
		pos.X = rc.Right;
	WriteConsoleOutputCharacter(pgui->m_std_output, &chBox[3], 1, pos, NULL);
}

int WriteInput(s_congui* pgui,TCHAR* psText )
{
	size_t len = _tcslen(psText);
	if (len == 0)
		return 0;
	int y = 0;
	INPUT_RECORD* piRec = (INPUT_RECORD*)malloc(sizeof(INPUT_RECORD)*((2*len)+1));
	INPUT_RECORD* pOneRec = piRec;
	pOneRec->EventType = FOCUS_EVENT;
	pOneRec->Event.FocusEvent.bSetFocus = TRUE;
	pOneRec++;
	int i;
	for (i=0; i < (2*len); i+=2) {
		pOneRec->EventType = KEY_EVENT;
		pOneRec->Event.KeyEvent.bKeyDown = TRUE;
		pOneRec->Event.KeyEvent.dwControlKeyState = 0;
		pOneRec->Event.KeyEvent.uChar.AsciiChar = psText[y];
		pOneRec->Event.KeyEvent.wRepeatCount = 1;
		pOneRec->Event.KeyEvent.wVirtualKeyCode = (psText[y]); /* virtual keycode is always uppercase */
		pOneRec->Event.KeyEvent.wVirtualScanCode = MapVirtualKeyA(pOneRec->Event.KeyEvent.wVirtualKeyCode & 0x00ff, 0);
		pOneRec++;
		pOneRec->EventType = KEY_EVENT;
		pOneRec->Event.KeyEvent.bKeyDown = FALSE;
		pOneRec->Event.KeyEvent.dwControlKeyState = 0;
		pOneRec->Event.KeyEvent.uChar.AsciiChar = psText[y];
		pOneRec->Event.KeyEvent.wRepeatCount = 1;
		pOneRec->Event.KeyEvent.wVirtualKeyCode = toupper(psText[y]);
		pOneRec->Event.KeyEvent.wVirtualScanCode = MapVirtualKeyA(pOneRec->Event.KeyEvent.wVirtualKeyCode & 0x00ff, 0);
		pOneRec++;
		y++;
	}
	DWORD pRed = 0;
	WriteConsoleInput(pgui->m_std_input,piRec,2*len,&pRed);
	free(piRec);
	return pRed;
}