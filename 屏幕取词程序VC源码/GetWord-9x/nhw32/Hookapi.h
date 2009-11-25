#ifndef _HOOKAPI_H_
#define _HOOKAPI_H_

#define HOOK_NEED_CHECK	0
#define HOOK_CAN_WRITE	1
#define HOOK_ONLY_READ	2

typedef struct _tagApiHookStruct
{
	LPSTR  lpszApiModuleName;
	LPSTR  lpszApiName;
	DWORD  dwApiOffset;
	LPVOID lpWinApiProc;
	BYTE   WinApiFiveByte[7];

	LPSTR  lpszHookApiModuleName;
	LPSTR  lpszHookApiName;
	LPVOID lpHookApiProc;
	BYTE   HookApiFiveByte[7];
	
	HINSTANCE hInst;

	BYTE   WinApiBakByte[7];
}
APIHOOKSTRUCT, *LPAPIHOOKSTRUCT;

FARPROC WINAPI getFunctionAddress(HINSTANCE hInst, LPCSTR lpMod, LPCSTR lpFun);
void MakeJMPCode(LPBYTE lpJMPCode, LPVOID lpCodePoint);
void MakeMemCanWrite(LPVOID lpMemPoint, BOOL bCanWrite, int nSysMemStatus);
void HookWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus);
void RestoreWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus);

////////////////////////////////////////////////////////////////////////////////
WORD __stdcall GetRing0Callgate( DWORD addr, unsigned cParams );
BOOL __stdcall FreeRing0Callgate( WORD callgate );
DWORD _SetPageAttributes(DWORD linear, DWORD dwAnd, DWORD dwOr);     // ASM version
DWORD SetPageAttributes(DWORD linear, DWORD dwAndParam, DWORD dwOrParam );

#endif // _HOOKAPI_H_
