// hookapi.h
#ifndef _INC_HOOKAPI
#define _INC_HOOKAPI

#include <windows.h>
#include "public.h"

#define PC_WRITEABLE	0x00020000
#define PC_USER			0x00040000
#define PC_STATIC		0x20000000

#define HOOK_NEED_CHECK 0
#define HOOK_CAN_WRITE	1
#define HOOK_ONLY_READ	2

#define BUFFERLEN		7

#define GETWORDEND_EVENT_NAME __TEXT("NH_GetWordEnd")	//added by ZHHN 1999.12.30
 
//Suijun: 模块说明
//Suijun: HOOK模块: 我们自己的模块，里面包含HOOK函数
//Suijun: API模块:  被HOOK的API所在的模块，比如gdi32.dll

typedef struct _tagApiHookStruct
{
	LPSTR  lpszApiModuleName; //Suijun: API模块的名称,比如gdi32.dll
	LPSTR  lpszApiName;       //Suijun: API模块中被HOOK的 API名称 
	DWORD  dwApiOffset;
	LPVOID lpWinApiProc;      //Suijun: API模块中被HOOK的函数地址    
	BYTE   WinApiFiveByte[7];      //Suijun: 保存原来API函数地址前7个字节的跳转指令值,主要用于卸载HOOK时候重新写入

	LPSTR  lpszHookApiModuleName; //Suijun: HOOK模块的名称
	LPSTR  lpszHookApiName;       //Suijun: HOOK模块中的目标函数(HOOK函数)名称
	LPVOID lpHookApiProc;         //Suijun: HOOK模块中HOOK函数的地址
	BYTE   HookApiFiveByte[7];    //Suijun: 保存我们HOOK的跳转指令
	
	HINSTANCE hInst;           //Suijun: 保存HOOK模块(即本模块)的句柄, 模块加载的起始地址,DLL装载时进行初始化,DllMain函数

	BYTE   WinApiBakByte[7];	//Suijun: 这玩意干啥用呢，不知道
}
APIHOOKSTRUCT, *LPAPIHOOKSTRUCT;

FARPROC WINAPI NHGetFuncAddress(HINSTANCE hInst, LPCSTR lpMod, LPCSTR lpFunc);
void MakeJMPCode(LPBYTE lpJMPCode, LPVOID lpCodePoint);
void MakeMemCanWrite(LPVOID lpMemPoint, BOOL bCanWrite, int nSysMemStatus);
void HookWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus);
void RestoreWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus);

#endif // _INC_HOOKAPI