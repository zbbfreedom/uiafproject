/////////////////////////////////////////////////////////////////////////
//
// hookapi.c
//
// Date   : 04/18/99
//
/////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "hookapi.h"
#include "public.h"
#include "string.h"

//#pragma comment(lib, "k32lib.lib")

//extern BOOL g_bCanWrite;

/////////////////////////////////////////////////////////////////////////
// Hook Api
/////////////////////////////////////////////////////////////////////////
//Suijun: 如果lpMod(模块名称)不为空，则从lpMod获取模块句柄，然后获取函数名为lpFunc的函数地址
//Suijun: 否则 直接从模块hInst中获取函数名为lpFunc的函数地址，
//Suijun: 失败，则返回空值

FARPROC WINAPI NHGetFuncAddress(HINSTANCE hInst, LPCSTR lpMod, LPCSTR lpFunc)
{
	HMODULE hMod;
	FARPROC procFunc;

	if (NULL != lpMod)
	{
		hMod=GetModuleHandle(lpMod);
		procFunc = GetProcAddress(hMod,lpFunc);
	}
	else
	{
		procFunc = GetProcAddress(hInst,lpFunc);

	}
	
	return  procFunc;
}

void MakeJMPCode(LPBYTE lpJMPCode, LPVOID lpCodePoint)
{
	BYTE temp;
	WORD wHiWord = HIWORD(lpCodePoint);
	WORD wLoWord = LOWORD(lpCodePoint);
	WORD wCS;

	_asm						// 讽玡匡拒才
	{
		push ax; //Suijun: 保存ax
		push cs; //Suijun: 保存cs
		pop  ax; //Suijun: 取cs到ax
		mov  wCS, ax;//Suijun:保存ax到wCS(内存中)
		pop  ax; //Suijun: 恢复ax址
	}; ////Suijun: 汇编，取出当前的cs寄存器(代码段寄存器)址
	
	lpJMPCode[0] = 0xea;		// 恶 JMP 诀竟絏
    
	temp = LOBYTE(wLoWord);		// -------------------------
	lpJMPCode[1] = temp;
	temp = HIBYTE(wLoWord);
	lpJMPCode[2] = temp;		// 恶ずい抖
	temp = LOBYTE(wHiWord);		// Point: 0x1234
	lpJMPCode[3] = temp;		// ず 4321
	temp = HIBYTE(wHiWord);
	lpJMPCode[4] = temp;		// -------------------------
	
	temp = LOBYTE(wCS);			// 恶匡拒才
	lpJMPCode[5] = temp;
	temp = HIBYTE(wCS);
	lpJMPCode[6] = temp;
    //Suijun: 跳转指令的构成，2个32bit的双字,
	//Suijun: 从最高字节7到0依次是：1个字节填充值0x0 + 2个字节的CS(代码段值) + 函数地址(4-1) + JMP跳转指令(0xEA)
	
	return;
}

//Suijun: HOOK API的实际执行函数 
void HookWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus)
{
	DWORD  dwReserved;
	DWORD  dwTemp;
	BYTE   bWin32Api[5];

	bWin32Api[0] = 0x00; 

	// 眔砆膁篒ㄧ计
	//Suijun: 如果API的地址为空值，则将其从gdi32.dll中模块中取出并保存, 例如 ::TextOutA
	//Suijun: 如果存在偏移地址则累加上偏移地址后保存
	if(lpApiHook->lpWinApiProc == NULL)
	{	
		lpApiHook->lpWinApiProc = (LPVOID)NHGetFuncAddress(lpApiHook->hInst, lpApiHook->lpszApiModuleName,lpApiHook->lpszApiName);
		if (lpApiHook->dwApiOffset != 0)
			lpApiHook->lpWinApiProc = (LPVOID)((DWORD)lpApiHook->lpWinApiProc + lpApiHook->dwApiOffset);
	}
	// 眔蠢ㄧ计
	//Suijun: 如果HOOK函数的目标地址为空，则从本模块中获取其地址，例如NHTextOutA
	//Suijun: 一般只需要计算1次
	if(lpApiHook->lpHookApiProc == NULL)
	{
		lpApiHook->lpHookApiProc = (LPVOID)NHGetFuncAddress(lpApiHook->hInst,
			lpApiHook->lpszHookApiModuleName,lpApiHook->lpszHookApiName);
	}
	// Θ JMP 
	//Suijun: 如果以前从没有计算新的跳转指令(跳转到我们的HOOKAPI), 则计算之并保存在HookApiFiveByte域中
	if (lpApiHook->HookApiFiveByte[0] == 0x00)
	{
		MakeJMPCode(lpApiHook->HookApiFiveByte, lpApiHook->lpHookApiProc);
	}

	//Suijun: 将API的目标函数的前16个字节从PAGE_EXECUTE_READ 更改为PAGE_READWRITE
	//Suijun: 一般代码段受保护的而不可写入的，为了修改它则必须更改内存模式为 可读写模式
	//Suijun: 此时将旧的保护模式存储在dwReserved,见后面调用同一个API进行恢复保护模式
	if (!VirtualProtect(lpApiHook->lpWinApiProc, 16, PAGE_READWRITE,
			&dwReserved))
	{
		MessageBox(NULL, "VirtualProtect-READWRITE", NULL, MB_OK);
		return;
	}
	
	//
	if (nSysMemStatus == HOOK_NEED_CHECK) 
	{
		//Suijun: 覆盖API函数原来的JMP指令，使其跳转到我们的函数，只覆盖了前7个字节就可以了
		memcpy(lpApiHook->lpWinApiProc, (LPVOID)lpApiHook->HookApiFiveByte,BUFFERLEN);
		
	}
	else
	{
		//Suijun: 判断是否保存了恢复时的JMP指令数据，如果没有则要保存一次原始API函数原来的跳转指令
		//Suijun: 该操作主要用于卸载HOOK时使用
		if (lpApiHook->WinApiFiveByte[0] == 0x00)			// 耞琌竒膁篒
		{
			// 
			// 称 API ㄧ计繷き竊
			memcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->lpWinApiProc,BUFFERLEN);
			// 耞琌狡膁篒(耞称繷き竊琌ΘJMP)
			//Suijun: 如果出现了恢复时的JMP指令数据 和 我们设置的HOOK跳转指令一样的情况，	
			//Suijun: 则更改跳转指令为0x33BFF3, 0X13FA15FF
			//Suijun: 这里不太明白该指令有何用途，我个人觉得不太可能出现这样的情况，
			if (strncmp(lpApiHook->WinApiFiveByte, lpApiHook->HookApiFiveByte, BUFFERLEN) == 0)
			{
				// 確称竊
				memcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->WinApiBakByte,BUFFERLEN);
			}
		}
		else
		{
			// 琌
		    //Suijun: 如果保存过了有效的 恢复时的JMP指令数据，则复制出来7个字节，个人认为应该复制出来7个字节
			//Suijun: 因为BUFFERLEN == 7,bWin32api只有5个字节，会发生写入非法
			memcpy(bWin32Api,(LPVOID)lpApiHook->lpWinApiProc,BUFFERLEN);
		}

		////Suijun: 如果此时API函数的前7个字节和我们的跳转指令并不相同，将我们的HOOK跳转指令写入 API函数的前7个字节，覆盖之
		if (strncmp(bWin32Api, lpApiHook->HookApiFiveByte, BUFFERLEN) != 0)
		{
			// 盢 JMP ﹚恶 API ㄧ计繷
			memcpy(lpApiHook->lpWinApiProc, (LPVOID)lpApiHook->HookApiFiveByte,BUFFERLEN);
		}
	}
    //Suijun: 恢复其原来的保护属性, PAGE_EXECUTE_READ
	if (!VirtualProtect(lpApiHook->lpWinApiProc, 16, dwReserved, &dwTemp))
	{
		MessageBox(NULL, "VirtualProtect-RESTORE", NULL, MB_OK);
		return;
	}
}

//Suijun: 卸载HOOK的具体执行函数, 请参考HookWin32Api函数
void RestoreWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus)
{
	DWORD dwReserved;
	DWORD dwTemp;

	if (lpApiHook->lpWinApiProc == NULL) //Suijun: 如果从未获取过Win32API的函数地址，不用恢复
		return;

	//Suijun: 修改前16个字节为可 读写模式，失败则返回
	if (!VirtualProtect(lpApiHook->lpWinApiProc, 16, PAGE_READWRITE,
			&dwReserved))
	{
		MessageBox(NULL, "VirtualProtect-READWRITE", NULL, MB_OK);
		return;
	}
	//Suijun: 将保存的原来API跳转指令重新写入到函数开始的7个字节出
	memcpy(lpApiHook->lpWinApiProc,(LPVOID)lpApiHook->WinApiFiveByte,BUFFERLEN);
	//Suijun: 恢复内存保护模式
	if (!VirtualProtect(lpApiHook->lpWinApiProc, 16, dwReserved, &dwTemp))
	{
		MessageBox(NULL, "VirtualProtect-RESTORE", NULL, MB_OK);
		return;
	}
}
