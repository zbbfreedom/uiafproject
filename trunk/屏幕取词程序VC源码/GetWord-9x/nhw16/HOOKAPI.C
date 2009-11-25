/**************************************************************************
    HookApi.C

    Hook and restore window api code in 16 bit system code.
    
    Hook   : Let application call api function after run my code.
    Restore: Let application call api function no run my code.

    (c) 1996.11 Inventec (TianJin) Co., Ltd.

    Author: FengShuen Lu / Gang Fang / Gang Yan

    Comments:  1. 97.5.30 is version 1.0.

***************************************************************************/
#include <dos.h>
#include <memory.h>
#include <string.h>
#include <windows.h>

#include "hookapi.h"

#ifdef _DEBUG
#include "DbgFunc.h"
#endif //_DEBUG

void myCpy(LPBYTE lpDest, LPBYTE lpSrc, UINT num);

// From a function name ,we get it's function address in special dll libary.
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

// Copy five byte form lpSrc to lpDest.
void byteFrompointer(BYTE* lpDest,LPVOID lpSrc)
{
	UINT seg = _FP_SEG(lpSrc);
	UINT off = _FP_OFF(lpSrc);
	BYTE seghigh = (BYTE)((seg&0xff00)>>8);
	BYTE seglow  = (BYTE)seg&0x00ff;
	BYTE offhigh = (BYTE)((off&0xff00)>>8);
	BYTE offlow  = (BYTE)off&0x00ff;

	lpDest ++;
	*lpDest = offlow;

	lpDest ++;
	*lpDest = offhigh;

	lpDest ++;
	*lpDest = seglow;

	lpDest ++;
	*lpDest = seghigh;

	return;
}

BYTE   bytWinTextout[5];

void HookWinApi(LPAPIHOOKSTRUCT lpApiHook, UINT nHookFlag)
{
	UINT   uSegCopy;
	LPVOID lpCopyToMem;

	bytWinTextout[0] = 0x00; 

	if(lpApiHook->lpWinApiProc == NULL)
	{	
		lpApiHook->lpWinApiProc = (LPVOID)getFunctionAddress(lpApiHook->hInst, lpApiHook->lpszApiModuleName,lpApiHook->lpszApiName);
		if (lpApiHook->dwApiOffset != 0)
			lpApiHook->lpWinApiProc = (LPVOID)((DWORD)lpApiHook->lpWinApiProc + lpApiHook->dwApiOffset);
	}
	if(lpApiHook->lpHookApiProc == NULL)
	{
		lpApiHook->lpHookApiProc = (LPVOID)getFunctionAddress(lpApiHook->hInst, lpApiHook->lpszHookApiModuleName,lpApiHook->lpszHookApiName);
	}

	if (lpApiHook->HookApiFiveByte[0] == 0x00)
	{
		lpApiHook->HookApiFiveByte[0]=0xea;    
		byteFrompointer(lpApiHook->HookApiFiveByte,lpApiHook->lpHookApiProc);
	}

	uSegCopy=AllocSelector((UINT)0);
	PrestoChangoSelector(_FP_SEG(lpApiHook->lpWinApiProc),uSegCopy);
	lpCopyToMem =_MK_FP(uSegCopy,_FP_OFF(lpApiHook->lpWinApiProc));
	
	switch(nHookFlag)
	{
		case HOOKANDBACK:
			 if (lpApiHook->WinApiFiveByte[0] == 0x00)
			 {
				_fmemcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpCopyToMem,5);
				if (strncmp(lpApiHook->WinApiFiveByte, lpApiHook->HookApiFiveByte, 5) == 0)
				{
					_fmemcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->WinApiBakByte,5);
				}
			 }
			 else
			 {
				_fmemcpy(bytWinTextout,(LPVOID)lpCopyToMem,5);
			 }
		
			 if (strncmp(bytWinTextout, lpApiHook->HookApiFiveByte, 5) != 0)
			 {
				_fmemcpy(lpCopyToMem,(LPVOID)lpApiHook->HookApiFiveByte,5);
			 }
			 break;
		case ONLYHOOK:
			 _fmemcpy(lpCopyToMem,(LPVOID)lpApiHook->HookApiFiveByte,5);
			 break;
	}
	
	FreeSelector(uSegCopy);
}

// copy old  textout code back.
void RestoreWinApi(LPAPIHOOKSTRUCT lpApiHook)
{
	UINT uSegCopy;
	LPVOID lpCopyToMem;

	if (lpApiHook->lpWinApiProc == NULL)
		return;

	uSegCopy=AllocSelector((UINT)0);
	PrestoChangoSelector(_FP_SEG(lpApiHook->lpWinApiProc),uSegCopy);
	lpCopyToMem =_MK_FP(uSegCopy,_FP_OFF(lpApiHook->lpWinApiProc));
	_fmemcpy(lpCopyToMem,(LPVOID)lpApiHook->WinApiFiveByte,5);
	FreeSelector(uSegCopy);
  
	return;
}
