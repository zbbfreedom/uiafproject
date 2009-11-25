/**************************************************************************
    HookApi.C

    Hook and restore window api code in 32 BIT system code.
    
    Hook   : Let application call api function after run my code.
    Restore: Let application call api function no run my code.

    (c) 1996.11 Inventec (TianJin) Co., Ltd.

    Author: FengShuen Lu / ZhenYu Hou / Gang Yan

    Comments:  1. 97.10.08 is version 1.0.

***************************************************************************/
#include <windows.h>
#include "string.h"
#include "hookapi.h"
#include "dbgfunc.h"

#pragma comment(lib, "k32lib.lib")

DWORD WINAPI VxDCall4(DWORD service_number, DWORD, DWORD, DWORD, DWORD);

#define PC_WRITEABLE	0x00020000
#define PC_USER			0x00040000
#define PC_STATIC		0x20000000

#define BUFFERLEN		7	// 用於定义一个长跳转的字节数·
BOOL g_bCanWrite = FALSE;

//////////////////
WORD callgate1 = 0;
/////////////////

// 用於取得指定模块中指定输出函数的地址·
FARPROC WINAPI getFunctionAddress(HINSTANCE hInst, LPCSTR lpMod, LPCSTR lpFun)
{
	HMODULE hMod;
	FARPROC procFun;

	if (lpMod != NULL)
	{
		hMod=GetModuleHandle(lpMod);
		procFun = GetProcAddress(hMod,lpFun);
	}
	else
	{
		procFun = GetProcAddress(hInst,lpFun);
	}
	
	return  procFun;
} 

// 用於形成一个３２ＢIT中的长跳转·
void MakeJMPCode(LPBYTE lpJMPCode, LPVOID lpCodePoint)
{
	BYTE temp;
	WORD wHiWord = HIWORD(lpCodePoint);
	WORD wLoWord = LOWORD(lpCodePoint);
	WORD wCS;

	_asm						// 取当前选择符·
	{
		push ax;
		push cs;
		pop  ax;
		mov  wCS, ax;
		pop  ax;
	};
	
	lpJMPCode[0] = 0xea;		// 填入 JMP 指令的机器码·

	temp = LOBYTE(wLoWord);		// -------------------------
	lpJMPCode[1] = temp;
	temp = HIBYTE(wLoWord);
	lpJMPCode[2] = temp;		// 填入地址·在内存中的顺序为；
	temp = LOBYTE(wHiWord);		// Point: 0x1234
	lpJMPCode[3] = temp;		// 内存： 4321
	temp = HIBYTE(wHiWord);
	lpJMPCode[4] = temp;		// -------------------------
	
	temp = LOBYTE(wCS);			// 填入选择符·
	lpJMPCode[5] = temp;
	temp = HIBYTE(wCS);
	lpJMPCode[6] = temp;

	return;
}

// 使指定指针处的内存可写/不可写·
void MakeMemCanWrite(LPVOID lpMemPoint, BOOL bCanWrite, int nSysMemStatus)
{
	
	switch (nSysMemStatus)
	{
		case HOOK_NEED_CHECK:
			 if (!g_bCanWrite)
			 {
				SetPageAttributes((DWORD)lpMemPoint, 0x0, 0x2);
				g_bCanWrite = TRUE;
			 }
			 break;
		case HOOK_CAN_WRITE:
			 SetPageAttributes((DWORD)lpMemPoint, 0x0, 0x2);
			 g_bCanWrite = TRUE;
			 break;
		case HOOK_ONLY_READ:
			 SetPageAttributes((DWORD)lpMemPoint, 0x42, 0x0);
			 g_bCanWrite = FALSE;
			 break;
	}

}

void HookWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus)
{											// nSysMemStatus = 0  说明需要检查可写标志·
											// nSysMemStatus = 1  说明需要设置状态为可写·
											// nSysMemStatus = 2  说明需要设置状态为只读·
	BYTE   bWin32Api[5];

	bWin32Api[0] = 0x00; 

	// 取得被拦截函数地址·
	if(lpApiHook->lpWinApiProc == NULL)
	{	
		lpApiHook->lpWinApiProc = (LPVOID)getFunctionAddress(lpApiHook->hInst, lpApiHook->lpszApiModuleName,lpApiHook->lpszApiName);
		if (lpApiHook->dwApiOffset != 0)
			lpApiHook->lpWinApiProc = (LPVOID)((DWORD)lpApiHook->lpWinApiProc + lpApiHook->dwApiOffset);
	}
	// 取得替代函数地址·
	if(lpApiHook->lpHookApiProc == NULL)
	{
		lpApiHook->lpHookApiProc = (LPVOID)getFunctionAddress(lpApiHook->hInst, lpApiHook->lpszHookApiModuleName,lpApiHook->lpszHookApiName);
	}
	// 形成 JMP 指令·
	if (lpApiHook->HookApiFiveByte[0] == 0x00)
	{
		MakeJMPCode(lpApiHook->HookApiFiveByte,lpApiHook->lpHookApiProc);
	}

	MakeMemCanWrite(lpApiHook->lpWinApiProc, TRUE, HOOK_CAN_WRITE);	// 令指定函数指针可写·
	
	if (nSysMemStatus == HOOK_NEED_CHECK)
	{
		memcpy(lpApiHook->lpWinApiProc, (LPVOID)lpApiHook->HookApiFiveByte,BUFFERLEN);
	}
	else
	{
		if (lpApiHook->WinApiFiveByte[0] == 0x00)			// 判断是否已经拦截·
		{
			// 否·
			// 备份 API 函数头五个字节·
			memcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->lpWinApiProc,BUFFERLEN);
			// 判断是否重复拦截·(即判断备份的头五个字节是否为形成的JMP指令)
			if (strncmp(lpApiHook->WinApiFiveByte, lpApiHook->HookApiFiveByte, BUFFERLEN) == 0)
			{
				// 恢复备份的字节·
				memcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->WinApiBakByte,BUFFERLEN);
			}
		}
		else
		{
			// 是·
			memcpy(bWin32Api,(LPVOID)lpApiHook->lpWinApiProc,BUFFERLEN);
		}

		if (strncmp(bWin32Api, lpApiHook->HookApiFiveByte, BUFFERLEN) != 0)
		{
			// 将 JMP 指定填入 API 函数的头·
			memcpy(lpApiHook->lpWinApiProc, (LPVOID)lpApiHook->HookApiFiveByte,BUFFERLEN);
		}
	}
	MakeMemCanWrite(lpApiHook->lpWinApiProc, FALSE, HOOK_ONLY_READ);	// 
}

void RestoreWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus)
{											// nSysMemStatus = 0  说明需要检查可写标志·
											// nSysMemStatus = 1  说明需要设置状态为可写·
											// nSysMemStatus = 2  说明需要设置状态为只读·

	if (lpApiHook->lpWinApiProc == NULL)
		return;

	MakeMemCanWrite(lpApiHook->lpWinApiProc, TRUE, HOOK_CAN_WRITE);	// 令指定函数指针可写·
	memcpy(lpApiHook->lpWinApiProc,(LPVOID)lpApiHook->WinApiFiveByte,BUFFERLEN);
	MakeMemCanWrite(lpApiHook->lpWinApiProc, FALSE, HOOK_ONLY_READ);
}

////////////////////////////////////////////////////////////////////////////////
//==================================
// PHYS - Matt Pietrek 1995
// FILE: PHYS.C
//==================================

DWORD SetPageAttributes(DWORD linear, DWORD dwAndParam, DWORD dwOrParam )
{
	
    WORD myFwordPtr[3];
    //if ( !callgate1 )
        callgate1 = GetRing0Callgate( (DWORD)_SetPageAttributes, 3 );
    
    if ( callgate1 )
    {
        WORD myFwordPtr[3];
        
        myFwordPtr[2] = callgate1;
        __asm   push    [linear]
		__asm   push    [dwOrParam]
		__asm   push    [dwAndParam]
        __asm   cli
        __asm   call    fword ptr [myFwordPtr]
        __asm   sti

        // The return value is in EAX.  The compiler will complain, but...
    }
    else
        return 0xFFFFFFFF;
	FreeRing0Callgate( callgate1 );
}
