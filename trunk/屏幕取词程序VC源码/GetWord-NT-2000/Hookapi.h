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
 
//Suijun: ģ��˵��
//Suijun: HOOKģ��: �����Լ���ģ�飬�������HOOK����
//Suijun: APIģ��:  ��HOOK��API���ڵ�ģ�飬����gdi32.dll

typedef struct _tagApiHookStruct
{
	LPSTR  lpszApiModuleName; //Suijun: APIģ�������,����gdi32.dll
	LPSTR  lpszApiName;       //Suijun: APIģ���б�HOOK�� API���� 
	DWORD  dwApiOffset;
	LPVOID lpWinApiProc;      //Suijun: APIģ���б�HOOK�ĺ�����ַ    
	BYTE   WinApiFiveByte[7];      //Suijun: ����ԭ��API������ַǰ7���ֽڵ���תָ��ֵ,��Ҫ����ж��HOOKʱ������д��

	LPSTR  lpszHookApiModuleName; //Suijun: HOOKģ�������
	LPSTR  lpszHookApiName;       //Suijun: HOOKģ���е�Ŀ�꺯��(HOOK����)����
	LPVOID lpHookApiProc;         //Suijun: HOOKģ����HOOK�����ĵ�ַ
	BYTE   HookApiFiveByte[7];    //Suijun: ��������HOOK����תָ��
	
	HINSTANCE hInst;           //Suijun: ����HOOKģ��(����ģ��)�ľ��, ģ����ص���ʼ��ַ,DLLװ��ʱ���г�ʼ��,DllMain����

	BYTE   WinApiBakByte[7];	//Suijun: �������ɶ���أ���֪��
}
APIHOOKSTRUCT, *LPAPIHOOKSTRUCT;

FARPROC WINAPI NHGetFuncAddress(HINSTANCE hInst, LPCSTR lpMod, LPCSTR lpFunc);
void MakeJMPCode(LPBYTE lpJMPCode, LPVOID lpCodePoint);
void MakeMemCanWrite(LPVOID lpMemPoint, BOOL bCanWrite, int nSysMemStatus);
void HookWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus);
void RestoreWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus);

#endif // _INC_HOOKAPI