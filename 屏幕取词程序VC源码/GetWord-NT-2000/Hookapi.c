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
//Suijun: ���lpMod(ģ������)��Ϊ�գ����lpMod��ȡģ������Ȼ���ȡ������ΪlpFunc�ĺ�����ַ
//Suijun: ���� ֱ�Ӵ�ģ��hInst�л�ȡ������ΪlpFunc�ĺ�����ַ��
//Suijun: ʧ�ܣ��򷵻ؿ�ֵ

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

	_asm						// ����e��ܲšO
	{
		push ax; //Suijun: ����ax
		push cs; //Suijun: ����cs
		pop  ax; //Suijun: ȡcs��ax
		mov  wCS, ax;//Suijun:����ax��wCS(�ڴ���)
		pop  ax; //Suijun: �ָ�axַ
	}; ////Suijun: ��࣬ȡ����ǰ��cs�Ĵ���(����μĴ���)ַ
	
	lpJMPCode[0] = 0xea;		// ��J JMP ���O�������X�O
    
	temp = LOBYTE(wLoWord);		// -------------------------
	lpJMPCode[1] = temp;
	temp = HIBYTE(wLoWord);
	lpJMPCode[2] = temp;		// ��J�a�}�O�b���s�������Ǭ��F
	temp = LOBYTE(wHiWord);		// Point: 0x1234
	lpJMPCode[3] = temp;		// ���s�G 4321
	temp = HIBYTE(wHiWord);
	lpJMPCode[4] = temp;		// -------------------------
	
	temp = LOBYTE(wCS);			// ��J��ܲšO
	lpJMPCode[5] = temp;
	temp = HIBYTE(wCS);
	lpJMPCode[6] = temp;
    //Suijun: ��תָ��Ĺ��ɣ�2��32bit��˫��,
	//Suijun: ������ֽ�7��0�����ǣ�1���ֽ����ֵ0x0 + 2���ֽڵ�CS(�����ֵ) + ������ַ(4-1) + JMP��תָ��(0xEA)
	
	return;
}

//Suijun: HOOK API��ʵ��ִ�к��� 
void HookWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus)
{
	DWORD  dwReserved;
	DWORD  dwTemp;
	BYTE   bWin32Api[5];

	bWin32Api[0] = 0x00; 

	// ���o�Q�d�I��Ʀa�}�O
	//Suijun: ���API�ĵ�ַΪ��ֵ�������gdi32.dll��ģ����ȡ��������, ���� ::TextOutA
	//Suijun: �������ƫ�Ƶ�ַ���ۼ���ƫ�Ƶ�ַ�󱣴�
	if(lpApiHook->lpWinApiProc == NULL)
	{	
		lpApiHook->lpWinApiProc = (LPVOID)NHGetFuncAddress(lpApiHook->hInst, lpApiHook->lpszApiModuleName,lpApiHook->lpszApiName);
		if (lpApiHook->dwApiOffset != 0)
			lpApiHook->lpWinApiProc = (LPVOID)((DWORD)lpApiHook->lpWinApiProc + lpApiHook->dwApiOffset);
	}
	// ���o���N��Ʀa�}�O
	//Suijun: ���HOOK������Ŀ���ַΪ�գ���ӱ�ģ���л�ȡ���ַ������NHTextOutA
	//Suijun: һ��ֻ��Ҫ����1��
	if(lpApiHook->lpHookApiProc == NULL)
	{
		lpApiHook->lpHookApiProc = (LPVOID)NHGetFuncAddress(lpApiHook->hInst,
			lpApiHook->lpszHookApiModuleName,lpApiHook->lpszHookApiName);
	}
	// �Φ� JMP ���O�O
	//Suijun: �����ǰ��û�м����µ���תָ��(��ת�����ǵ�HOOKAPI), �����֮��������HookApiFiveByte����
	if (lpApiHook->HookApiFiveByte[0] == 0x00)
	{
		MakeJMPCode(lpApiHook->HookApiFiveByte, lpApiHook->lpHookApiProc);
	}

	//Suijun: ��API��Ŀ�꺯����ǰ16���ֽڴ�PAGE_EXECUTE_READ ����ΪPAGE_READWRITE
	//Suijun: һ�������ܱ����Ķ�����д��ģ�Ϊ���޸������������ڴ�ģʽΪ �ɶ�дģʽ
	//Suijun: ��ʱ���ɵı���ģʽ�洢��dwReserved,���������ͬһ��API���лָ�����ģʽ
	if (!VirtualProtect(lpApiHook->lpWinApiProc, 16, PAGE_READWRITE,
			&dwReserved))
	{
		MessageBox(NULL, "VirtualProtect-READWRITE", NULL, MB_OK);
		return;
	}
	
	//
	if (nSysMemStatus == HOOK_NEED_CHECK) 
	{
		//Suijun: ����API����ԭ����JMPָ�ʹ����ת�����ǵĺ�����ֻ������ǰ7���ֽھͿ�����
		memcpy(lpApiHook->lpWinApiProc, (LPVOID)lpApiHook->HookApiFiveByte,BUFFERLEN);
		
	}
	else
	{
		//Suijun: �ж��Ƿ񱣴��˻ָ�ʱ��JMPָ�����ݣ����û����Ҫ����һ��ԭʼAPI����ԭ������תָ��
		//Suijun: �ò�����Ҫ����ж��HOOKʱʹ��
		if (lpApiHook->WinApiFiveByte[0] == 0x00)			// �P�_�O�_�w�g�d�I�O
		{
			// �_�O
			// �ƥ� API ����Y���Ӧr�`�O
			memcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->lpWinApiProc,BUFFERLEN);
			// �P�_�O�_�����d�I�O(�Y�P�_�ƥ����Y���Ӧr�`�O�_���Φ���JMP���O)
			//Suijun: ��������˻ָ�ʱ��JMPָ������ �� �������õ�HOOK��תָ��һ���������	
			//Suijun: �������תָ��Ϊ0x33BFF3, 0X13FA15FF
			//Suijun: ���ﲻ̫���׸�ָ���к���;���Ҹ��˾��ò�̫���ܳ��������������
			if (strncmp(lpApiHook->WinApiFiveByte, lpApiHook->HookApiFiveByte, BUFFERLEN) == 0)
			{
				// ��_�ƥ����r�`�O
				memcpy(lpApiHook->WinApiFiveByte,(LPVOID)lpApiHook->WinApiBakByte,BUFFERLEN);
			}
		}
		else
		{
			// �O�O
		    //Suijun: ������������Ч�� �ָ�ʱ��JMPָ�����ݣ����Ƴ���7���ֽڣ�������ΪӦ�ø��Ƴ���7���ֽ�
			//Suijun: ��ΪBUFFERLEN == 7,bWin32apiֻ��5���ֽڣ��ᷢ��д��Ƿ�
			memcpy(bWin32Api,(LPVOID)lpApiHook->lpWinApiProc,BUFFERLEN);
		}

		////Suijun: �����ʱAPI������ǰ7���ֽں����ǵ���תָ�����ͬ�������ǵ�HOOK��תָ��д�� API������ǰ7���ֽڣ�����֮
		if (strncmp(bWin32Api, lpApiHook->HookApiFiveByte, BUFFERLEN) != 0)
		{
			// �N JMP ���w��J API ��ƪ��Y�O
			memcpy(lpApiHook->lpWinApiProc, (LPVOID)lpApiHook->HookApiFiveByte,BUFFERLEN);
		}
	}
    //Suijun: �ָ���ԭ���ı�������, PAGE_EXECUTE_READ
	if (!VirtualProtect(lpApiHook->lpWinApiProc, 16, dwReserved, &dwTemp))
	{
		MessageBox(NULL, "VirtualProtect-RESTORE", NULL, MB_OK);
		return;
	}
}

//Suijun: ж��HOOK�ľ���ִ�к���, ��ο�HookWin32Api����
void RestoreWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus)
{
	DWORD dwReserved;
	DWORD dwTemp;

	if (lpApiHook->lpWinApiProc == NULL) //Suijun: �����δ��ȡ��Win32API�ĺ�����ַ�����ûָ�
		return;

	//Suijun: �޸�ǰ16���ֽ�Ϊ�� ��дģʽ��ʧ���򷵻�
	if (!VirtualProtect(lpApiHook->lpWinApiProc, 16, PAGE_READWRITE,
			&dwReserved))
	{
		MessageBox(NULL, "VirtualProtect-READWRITE", NULL, MB_OK);
		return;
	}
	//Suijun: �������ԭ��API��תָ������д�뵽������ʼ��7���ֽڳ�
	memcpy(lpApiHook->lpWinApiProc,(LPVOID)lpApiHook->WinApiFiveByte,BUFFERLEN);
	//Suijun: �ָ��ڴ汣��ģʽ
	if (!VirtualProtect(lpApiHook->lpWinApiProc, 16, dwReserved, &dwTemp))
	{
		MessageBox(NULL, "VirtualProtect-RESTORE", NULL, MB_OK);
		return;
	}
}
