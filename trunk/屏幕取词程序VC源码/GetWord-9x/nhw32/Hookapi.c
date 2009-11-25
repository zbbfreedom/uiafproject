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

#define BUFFERLEN		7	// ��춶���һ������ת���ֽ�����
BOOL g_bCanWrite = FALSE;

//////////////////
WORD callgate1 = 0;
/////////////////

// ���ȡ��ָ��ģ����ָ����������ĵ�ַ��
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

// ����γ�һ��������IT�еĳ���ת��
void MakeJMPCode(LPBYTE lpJMPCode, LPVOID lpCodePoint)
{
	BYTE temp;
	WORD wHiWord = HIWORD(lpCodePoint);
	WORD wLoWord = LOWORD(lpCodePoint);
	WORD wCS;

	_asm						// ȡ��ǰѡ�����
	{
		push ax;
		push cs;
		pop  ax;
		mov  wCS, ax;
		pop  ax;
	};
	
	lpJMPCode[0] = 0xea;		// ���� JMP ָ��Ļ����롤

	temp = LOBYTE(wLoWord);		// -------------------------
	lpJMPCode[1] = temp;
	temp = HIBYTE(wLoWord);
	lpJMPCode[2] = temp;		// �����ַ�����ڴ��е�˳��Ϊ��
	temp = LOBYTE(wHiWord);		// Point: 0x1234
	lpJMPCode[3] = temp;		// �ڴ棺 4321
	temp = HIBYTE(wHiWord);
	lpJMPCode[4] = temp;		// -------------------------
	
	temp = LOBYTE(wCS);			// ����ѡ�����
	lpJMPCode[5] = temp;
	temp = HIBYTE(wCS);
	lpJMPCode[6] = temp;

	return;
}

// ʹָ��ָ�봦���ڴ��д/����д��
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
{											// nSysMemStatus = 0  ˵����Ҫ����д��־��
											// nSysMemStatus = 1  ˵����Ҫ����״̬Ϊ��д��
											// nSysMemStatus = 2  ˵����Ҫ����״̬Ϊֻ����
	BYTE   bWin32Api[5];

	bWin32Api[0] = 0x00; 

	// ȡ�ñ����غ�����ַ��
	if(lpApiHook->lpWinApiProc == NULL)
	{	
		lpApiHook->lpWinApiProc = (LPVOID)getFunctionAddress(lpApiHook->hInst, lpApiHook->lpszApiModuleName,lpApiHook->lpszApiName);
		if (lpApiHook->dwApiOffset != 0)
			lpApiHook->lpWinApiProc = (LPVOID)((DWORD)lpApiHook->lpWinApiProc + lpApiHook->dwApiOffset);
	}
	// ȡ�����������ַ��
	if(lpApiHook->lpHookApiProc == NULL)
	{
		lpApiHook->lpHookApiProc = (LPVOID)getFunctionAddress(lpApiHook->hInst, lpApiHook->lpszHookApiModuleName,lpApiHook->lpszHookApiName);
	}
	// �γ� JMP ָ�
	if (lpApiHook->HookApiFiveByte[0] == 0x00)
	{
		MakeJMPCode(lpApiHook->HookApiFiveByte,lpApiHook->lpHookApiProc);
	}

	MakeMemCanWrite(lpApiHook->lpWinApiProc, TRUE, HOOK_CAN_WRITE);	// ��ָ������ָ���д��
	
	if (nSysMemStatus == HOOK_NEED_CHECK)
	{
		memcpy(lpApiHook->lpWinApiProc, (LPVOID)lpApiHook->HookApiFiveByte,BUFFERLEN);
	}
	else
	{
		if (lpApiHook->WinApiFiveByte[0] == 0x00)			// �ж��Ƿ��Ѿ����ء�
		{
			// ��
			// ���� API ����ͷ����ֽڡ�
			memcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->lpWinApiProc,BUFFERLEN);
			// �ж��Ƿ��ظ����ء�(���жϱ��ݵ�ͷ����ֽ��Ƿ�Ϊ�γɵ�JMPָ��)
			if (strncmp(lpApiHook->WinApiFiveByte, lpApiHook->HookApiFiveByte, BUFFERLEN) == 0)
			{
				// �ָ����ݵ��ֽڡ�
				memcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->WinApiBakByte,BUFFERLEN);
			}
		}
		else
		{
			// �ǡ�
			memcpy(bWin32Api,(LPVOID)lpApiHook->lpWinApiProc,BUFFERLEN);
		}

		if (strncmp(bWin32Api, lpApiHook->HookApiFiveByte, BUFFERLEN) != 0)
		{
			// �� JMP ָ������ API ������ͷ��
			memcpy(lpApiHook->lpWinApiProc, (LPVOID)lpApiHook->HookApiFiveByte,BUFFERLEN);
		}
	}
	MakeMemCanWrite(lpApiHook->lpWinApiProc, FALSE, HOOK_ONLY_READ);	// 
}

void RestoreWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus)
{											// nSysMemStatus = 0  ˵����Ҫ����д��־��
											// nSysMemStatus = 1  ˵����Ҫ����״̬Ϊ��д��
											// nSysMemStatus = 2  ˵����Ҫ����״̬Ϊֻ����

	if (lpApiHook->lpWinApiProc == NULL)
		return;

	MakeMemCanWrite(lpApiHook->lpWinApiProc, TRUE, HOOK_CAN_WRITE);	// ��ָ������ָ���д��
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
