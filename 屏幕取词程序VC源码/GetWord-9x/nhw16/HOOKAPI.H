#ifndef _HOOKAPI_H_
#define _HOOKAPI_H_

#define HOOKANDBACK	0
#define ONLYHOOK	1

typedef struct _tagApiHookStruct
{
	LPSTR  lpszApiModuleName;
	LPSTR  lpszApiName;
	DWORD  dwApiOffset;
	LPVOID lpWinApiProc;
	BYTE   WinApiFiveByte[5];

	LPSTR  lpszHookApiModuleName;
	LPSTR  lpszHookApiName;
	LPVOID lpHookApiProc;
	BYTE   HookApiFiveByte[5];
	
	HINSTANCE hInst;

	BYTE   WinApiBakByte[5];
}
APIHOOKSTRUCT, *LPAPIHOOKSTRUCT;

FARPROC WINAPI getFunctionAddress(HINSTANCE hInst, LPCSTR lpMod, LPCSTR lpFun);
void byteFrompointer(BYTE* lpDest,LPVOID lpSrc);
void HookWinApi(LPAPIHOOKSTRUCT lpApiHook, UINT nHookFlag);
void RestoreWinApi(LPAPIHOOKSTRUCT lpApiHook);

#endif // _HOOKAPI_H_
