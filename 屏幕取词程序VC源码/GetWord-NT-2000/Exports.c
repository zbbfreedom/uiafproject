/////////////////////////////////////////////////////////////////////////
//
// exports.c
//
// Author : Chen Shuqing
//
// Date   : 04/18/99
//
// Fix Bug: Zhang Haining
//
// Date   : 01/17/2000
// ����������������������õ��Ľṹ�塣
/////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "findword.h"
#include "exports.h"
#include "hookapi.h"
#include "public.h"
#include "dbgprint.h"

//Suijun: ����Ϊ���н��̹�������ݶΣ�ÿ������װ�ظ�DLL�Ľ��̶�����
//Suijun: ���޸Ķ���������Ҳ�ᷢ��Ӱ��
//LJM: Demo���������̶������NWH32.DLL,Ȼ��Ṳ��ʹ���������,����ÿ�ζ�������Щ����.
#pragma data_seg(".sdata") //Start �������ݶ�

UINT g_nFlag = 0;	//must share
HWND g_hNotifyWnd = NULL;	//must share
int g_MouseX = 0;	//must share
int g_MouseY = 0;	//must share
BOOL g_bNewGetWordFlag = FALSE;		//must share

char g_szTotalWord[BUFFERLENGTH] = "";	//must share
RECT g_TotalWordRect = {0,0,0,0};			// �Ω�O����������ϰ�j�p�Omust share
//��ǰ��������ǲ��Ǳ���ȡ�������д�
int  g_bMouseInTotalWord = FALSE;           // �Ω�O�����ЬO�_�b��������Omust share
int  g_nCurCaretPlaceInTotalWord = -1;		// �Ω�O�����Цb�����������m�Omust share
RECT g_rcFirstWordRect = {0,0,0,0};	//must share

int g_nGetWordStyle = 0;	//must share
int g_nWordsInPhrase = -1;	//must share
BOOL g_bPhraseNeeded = FALSE;	//must share

BOOL g_bHooked = FALSE;	//must share

//int g_nProcessHooked = 0;		//must share

char szMemDCWordBuff[BUFFERLENGTH] = "";	// �Ω�O���Ҧ� MemDC ���� Text �奻�O
int  pnMemDCCharLeft[BUFFERLENGTH];			// �Ω�O���b TextOut ���Ҧ��r�����۹�ȡO
int  pnMemDCCharRight[BUFFERLENGTH];		// �Ω�O���b TextOut ���Ҧ��r���k�۹�ȡO
WORDPARA WordBuffer[MEMDC_MAXNUM];			// �Ω�O���b TextOut ��������Ҧ������H���O
int nWordNum = 0;							// �O�� MemDC ��������ӼơO

#pragma data_seg()	//End �������ݶ���������

UINT g_uMsg = 0;

BOOL g_bOldGetWordFlag = FALSE;

HWND g_hWndParent = NULL;	//Ŀ�괰������

BOOL  g_bAllowGetCurWord = FALSE;	//�Ƿ������ȡ��ǰ�ı�

int  g_CharType = CHAR_TYPE_OTHER;			// char���ͣ�Ĭ��Ϊ����ASCII��Ҳ���Ǻ��֣�

// ��e���ƾڵ��c�O( ��e���G���ѿ�J�w�İϤ����X����� )
char g_szCurWord[WORDMAXLEN] = "";
RECT g_CurWordRect = {0,0,0,0};
int  g_nCurCaretPlace = 0;
POINT g_CurMousePos = {0,0};

UINT         g_nTextAlign = 0;
POINT        g_dwDCOrg = {0,0};
int          g_nExtra = 0;
POINT        g_CurPos = {0,0};
TEXTMETRIC   g_tm;

BOOL bRecAllRect = TRUE;
RECT g_rcTotalRect = {0,0,0,0};

UINT BL_HASSTRING = 0;

int g_nPhraseCharType = CHAR_TYPE_OTHER;

HHOOK g_hHook = NULL;	//can not share else calling UnhookWindowsHookEx() will occur error
HINSTANCE g_hinstDll = NULL;	//
HANDLE hMutex = NULL;		//�߳�ͬ���ź�

//����GetMessage�Ļص�����
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
//UINCODE�ַ�����ANSI��ת������
LPSTR UnicodeToAnsi(LPTSTR lpString, UINT cbCount);

//���������ҪHook API�����ݽṹ Start
//ΪʲôҪHook�����ǰ��̫���
APIHOOKSTRUCT g_BitBltHook = {
	"gdi32.dll",
	"BitBlt",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHBitBlt",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

APIHOOKSTRUCT g_TextOutAHook = {
	"gdi32.dll",
	"TextOutA",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHTextOutA",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

APIHOOKSTRUCT g_TextOutWHook = {
	"gdi32.dll",
	"TextOutW",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHTextOutW",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

APIHOOKSTRUCT g_ExtTextOutAHook = {
	"gdi32.dll",
	"ExtTextOutA",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHExtTextOutA",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

APIHOOKSTRUCT g_ExtTextOutWHook = {
	"gdi32.dll",
	"ExtTextOutW",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHExtTextOutW",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};
//���϶������Hook Api�Ľṹ�� End

//LJM: �������ݶε�˵���������г���ɶ�,��д
#pragma comment(linker,"-section:.sdata,rws")

////////////////////////////////////////////////////////////////////////////////
//
//	DllMain()
//  DLL �������[��DLL�ᱻDemo.exe��Ŀ����̼���]
////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) 
	{
		case DLL_PROCESS_ATTACH:	//���̼��ظ�DLLʱ
			
			g_hinstDll = hinstDLL;	//�洢�ý��̵ľ������g_hinstDll��[Demo.exe]
									//�����Hook�ṹ���е�hInst��Ϊ��ǰ����
			g_BitBltHook.hInst = hinstDLL;
			g_TextOutAHook.hInst = hinstDLL;
			g_TextOutWHook.hInst = hinstDLL;
			g_ExtTextOutAHook.hInst = hinstDLL;
			g_ExtTextOutWHook.hInst = hinstDLL;
			
			//ע��Ϊ���̼�ͨ��ע��windows��Ϣ-----------??��֪��ΪʲôҪע��"Noble Hand"
			g_uMsg = RegisterWindowMessage("Noble Hand");
			if(!g_uMsg)
			{
				return FALSE;
			}

			//Added by ZHHN on 2000.2.2
			//Because forget to add this function before, with the result that it gets word
			//little slowly
			//ע��Ϊ���̼�ͨ��ע��windows��Ϣ-----------???
			BL_HASSTRING = RegisterWindowMessage(MSG_HASSTRINGNAME);
			if(!BL_HASSTRING)
			{
				return FALSE;
			}

			// create mutex
			//����һ����
			hMutex = CreateMutex(NULL, FALSE, MUTEXNAME);
			if (NULL == hMutex)
			{
				return FALSE;
			}
			break;

		case DLL_THREAD_ATTACH:	
			 break;

		case DLL_THREAD_DETACH:
			 break;
		
		case DLL_PROCESS_DETACH:	//����ж�ظ�DLLʱ

			// restore �ָ�
			NHUnHookWin32Api(); //Suijun: ����HOOK�ж�

			if (NULL != hMutex)
			{
				CloseHandle(hMutex);
			}

			break;
    }

    return TRUE;
}

//��������,�൱�ں꣬�������,��ΪDLLһ�����أ�ϵͳ�е�GetMessage��������
//GetMessage�����Ļص����� ����ϵͳ��GetMessage����
__inline void ProcessWin32API()
{
	if (g_bNewGetWordFlag != g_bOldGetWordFlag)
	{
		if (g_bNewGetWordFlag)//�Ƿ�����˳�ʼ����BL_SetFlag32����Ϊȡ��
		{
			WaitForSingleObject(hMutex, INFINITE); //Suijun: �ȴ��첽�źţ�ȷ�����´���ֻ��1������ִ��

			g_nGetWordStyle = g_nFlag;
			g_bAllowGetCurWord = TRUE;
			g_CurMousePos.x = g_MouseX;
			g_CurMousePos.y = g_MouseY;
			g_szCurWord[0] = 0x00;
			g_nCurCaretPlace = -1;

			if (!g_bHooked)	//�Ƿ��Ѿ�Hook�ɹ�//��һ�����ڻ�û��Hook,�����ifѭ����
			{
				NHHookWin32Api();	//û��Hook������Hook
				g_bHooked = TRUE;	//����Hook�ɹ���ʶ
			}

			g_bAllowGetCurWord = TRUE;

			ReleaseMutex(hMutex);//�ͷŶ�ռ��
		}
		else
		{	//ͨ��BL_SetFlag32����g_bNewGetWordFlag=false ȡ��ȡ��
			WaitForSingleObject(hMutex, INFINITE);

			g_bAllowGetCurWord = FALSE;

			if (g_bHooked)
			{
				RestoreWin32Api(&g_BitBltHook, HOOK_ONLY_READ);
				RestoreWin32Api(&g_TextOutAHook, HOOK_ONLY_READ);
				RestoreWin32Api(&g_TextOutWHook, HOOK_ONLY_READ);
				RestoreWin32Api(&g_ExtTextOutAHook, HOOK_ONLY_READ);
				RestoreWin32Api(&g_ExtTextOutWHook, HOOK_ONLY_READ);
				g_bHooked = FALSE;
			}

			ReleaseMutex(hMutex);
		}
	}

	g_bOldGetWordFlag = g_bNewGetWordFlag;
}

//DLL �������
//ȡ�ʳ�ʼ��(ÿ��ȡ�ʶ���Ҫ����)
DLLEXPORT DWORD WINAPI BL_SetFlag32(UINT nFlag, HWND hNotifyWnd,
									int MouseX, int MouseY)
{
	POINT ptWindow;
	HWND hWnd;
	//char classname[256];
	DWORD dwCurrProcessId = 0;
	DWORD dwProcessIdOfWnd = 0;

/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "%s\n", "BL_SetFlag32");
		OutputDebugString(cBuffer);
	}
*/
	g_nFlag = nFlag;
	g_hNotifyWnd = hNotifyWnd;
	g_MouseX = MouseX;
	g_MouseY = MouseY;

	ptWindow.x = MouseX;
	ptWindow.y = MouseY;
	//��ð���ָ����Ĵ��ڵľ��,���Ҳ�Ǻ�����ҪHook�Ĵ��ڽ���
	hWnd = WindowFromPoint(ptWindow);

	g_hWndParent = hWnd;

	switch (nFlag)
	{
//		case GETWORD_ENABLE:
		case GETWORD_D_ENABLE:
		case GETWORD_TW_ENABLE:
		case GETPHRASE_ENABLE:	//����ȡ��

			g_bNewGetWordFlag = TRUE;

			BL_SetGetWordStyle(GETPHRASE_ENABLE);

			dwCurrProcessId = GetCurrentProcessId();	//��ǰ����ID
			//��õ�ǰȡ��Ŀ�����ID
			GetWindowThreadProcessId(g_hWndParent, &dwProcessIdOfWnd);

			if(dwProcessIdOfWnd == dwCurrProcessId)	//�����ǰHook�Ľ��̺�Ŀ�괰�ڽ���һ��,��ʼ����
			{
				ProcessWin32API();	//�ֶ�ִ��Hook����,����ȡ��
			}
			else
			{				
				SendMessage(g_hWndParent, g_uMsg, 0, 0);	//���ŷ���һ��
				PostMessage(g_hWndParent, g_uMsg, 0, 0);			
			}

			break;

		case GETWORD_D_TYPING_ENABLE:

			g_bNewGetWordFlag = TRUE;

			BL_SetGetWordStyle(GETPHRASE_DISABLE);

			PostMessage(g_hWndParent, g_uMsg, 0, 0);			

			break;

		case GETWORD_DISABLE:
		case GETPHRASE_DISABLE:

			g_bNewGetWordFlag = FALSE;

			dwCurrProcessId = GetCurrentProcessId();
			GetWindowThreadProcessId(g_hWndParent, &dwProcessIdOfWnd);

			if(dwProcessIdOfWnd == dwCurrProcessId)
			{
				ProcessWin32API();
			}
			else
			{
				SendMessage(g_hWndParent, g_uMsg, 0, 0);
				PostMessage(g_hWndParent, g_uMsg, 0, 0);			
			}

			break;

		case GETWORD_D_TYPING_DISABLE:

			g_bNewGetWordFlag = FALSE;

			PostMessage(g_hWndParent, g_uMsg, 0, 0);			

			break;

		default:
			break;
	}

	// Fix bug2 end

	return BL_OK;
}

//Hook ����Win32Api
DLLEXPORT DWORD WINAPI NHHookWin32Api()
{
	//��һЩȡ��ǰ��������(��֮ǰȡ���������)
	g_nCurCaretPlace = -1;
	g_szCurWord[0] = 0x00;
	g_szTotalWord[0] = 0x00;
	g_nCurCaretPlaceInTotalWord = -1;
	g_CharType = CHAR_TYPE_OTHER;
	g_bMouseInTotalWord = FALSE;
	g_bAllowGetCurWord = TRUE;

	nWordNum = 0;
	szMemDCWordBuff[0] = 0x00;

	//��ʼHook���е������������
	HookAllTextOut();

	return BL_OK;
}

//����UnHookAllTextOut() ж������HOOK
DLLEXPORT DWORD WINAPI NHUnHookWin32Api()
{
	g_bAllowGetCurWord = FALSE;
	UnHookAllTextOut();
        
	return BL_OK;
}

//Suijun: ��װHOOK, HOOK  5�� Windows API����BitBlt, TextOutA, TextOutW, ExtTextOutA, ExtTextOutW
void HookAllTextOut()
{
	HookWin32Api(&g_BitBltHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_TextOutAHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_TextOutWHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_ExtTextOutAHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_ExtTextOutWHook, HOOK_CAN_WRITE);
}

//Suijun: ж������HOOK
void UnHookAllTextOut()
{
	RestoreWin32Api(&g_BitBltHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_TextOutAHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_TextOutWHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_ExtTextOutAHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);
}

//Hook��ͼƬ�ϻ��ֵĹ���
DLLEXPORT BOOL WINAPI NHBitBlt(HDC hdcDest,
						       int nXDest,
						       int nYDest,
						       int nWidth,
						       int nHeight,
						       HDC hdcSrc,
						       int nXSrc,
						       int nYSrc,
						       DWORD dwRop)
{
	int x, y;

	POINT pt;
	HWND  hWDC;
	HWND  hWPT;
	//char lpClassName[256];
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHBitBlt : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("-> NHBitBlt : %s\n", "start");

	// restore
	RestoreWin32Api(&g_BitBltHook, HOOK_NEED_CHECK);

	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdcDest);
	hWPT = WindowFromPoint(pt);	

	if (hWPT == hWDC) //added by zhhn 01/17/2000
	{
		if (nWidth > 5)	//Xianfeng:����5�����ܷ����ַ�
		{
				GetDCOrgEx(hdcDest, &g_dwDCOrg);
				x = g_dwDCOrg.x;
				y = g_dwDCOrg.y;
				x += nXDest;
				y += nYDest;
				g_dwDCOrg.x = x;
				g_dwDCOrg.y = y;
    
				CheckMemDCWordBuffer(hdcDest, hdcSrc);
		}
		else
		{
			if (CheckDCWndClassName(hdcDest))
			{
				GetDCOrgEx(hdcDest, &g_dwDCOrg);
				x = g_dwDCOrg.x;
				y = g_dwDCOrg.y;
				x += nXDest;
				y += nYDest;
				g_dwDCOrg.x = x;
				g_dwDCOrg.y = y;
    
				CheckMemDCWordBuffer(hdcDest, hdcSrc);
			}
		}
	}

	// call BitBlt
	BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight,
		   hdcSrc, nXSrc, nYSrc, dwRop);

	HookWin32Api(&g_BitBltHook, HOOK_NEED_CHECK);

	return TRUE;
}

//Hook TextOutA�Ĺ���
DLLEXPORT BOOL WINAPI NHTextOutA(HDC hdc,
							     int nXStart,
							     int nYStart,
							     LPCTSTR lpString,
							     int cbString)
{
	POINT pt;
	HWND  hWDC;
	HWND  hWPT;
	DWORD dwThreadIdWithPoint = 0;
	DWORD dwThreadIdCurr = 0;

/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHTextOutA : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/

	//DbgFilePrintf("-> NHTextOutA : lpString(%s), cbString(%d)\n", lpString, cbString);

	// restore
	RestoreWin32Api(&g_TextOutAHook, HOOK_NEED_CHECK);

	//
	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdc);	//Xianfeng:�Է����ڴ�����
	hWPT = WindowFromPoint(pt);	//Xianfeng:������ڴ�����

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);	//Xianfeng:ȡ��������ڴ����߳�ID
	dwThreadIdCurr = GetCurrentThreadId();		//Xianfeng:ȡ�õ�ǰ�߳�ID

	if(dwThreadIdWithPoint == dwThreadIdCurr)
	{
		if (hWDC == NULL 
			|| hWPT == hWDC
			|| IsParentOrSelf(hWPT, hWDC)	
			|| IsParentOrSelf(hWDC, hWPT))
		{
			if ((g_bAllowGetCurWord) 
				&& (!IsBadReadPtr(lpString, cbString))
				&& (cbString > 0))
			{
					g_nTextAlign = GetTextAlign(hdc);			//Xianfeng:ȡ�õ�ǰDC���ı����뷽ʽ���ں�����������
					g_nExtra     = GetTextCharacterExtra(hdc);	//Xianfeng:ȡ���ַ����϶����λ����������
					GetCurrentPositionEx(hdc, &g_CurPos);		//Xianfeng:ȡ�õ�ǰDC���߼�λ��
					GetTextMetrics(hdc, &g_tm);					//Xianfeng:ȡ�õ�ǰDC��font�Ļ�����Ϣ
        
					g_dwDCOrg.x = 0;
					g_dwDCOrg.y = 0;
					bRecAllRect = FALSE;
					GetStringRect(hdc, (LPSTR)lpString, cbString, nXStart,
								  nYStart, &g_rcTotalRect, NULL);
					bRecAllRect = TRUE;					

					if ((WindowFromDC != NULL)&&(WindowFromDC(hdc) == NULL))
					{
							g_dwDCOrg.x = 0;
							g_dwDCOrg.y = 0;

							//Xianfeng:��������ַ���Ϣ���ڴ�
							AddToTextOutBuffer(hdc, (LPSTR)lpString, cbString,
											   nXStart, nYStart, NULL);
					}
					else
					{
							//Xianfeng:ȡ��DCԭ�㣬ͨ����ֵ�ǿͻ������Ͻ�����ڴ������Ͻǵ�ƫ����
							GetDCOrgEx(hdc, &g_dwDCOrg);	
        
							//Xianfeng:��ȡ��ǰ����µ���
							GetCurMousePosWord(hdc, (LPSTR)lpString, cbString,
											   nXStart, nYStart, NULL);
					}
			}
		}
	}

	// call TextOutA
	TextOutA(hdc, nXStart, nYStart, lpString, cbString);

	HookWin32Api(&g_TextOutAHook, HOOK_NEED_CHECK);

	return TRUE;
}

//Hook TextOutW �Ĺ���
DLLEXPORT BOOL WINAPI NHTextOutW(HDC hdc,
							     int nXStart,
							     int nYStart,
							     LPCWSTR lpString,
							     int cbString)
{
	POINT pt;
	HWND  hWDC;
	HWND  hWPT;
	DWORD dwThreadIdWithPoint = 0;
	DWORD dwThreadIdCurr = 0;

/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHTextOutW : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("-> NHTextOutW : lpString(%s), cbString(%d)\n", lpString, cbString);

	// restore
	RestoreWin32Api(&g_TextOutWHook, HOOK_NEED_CHECK);

	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdc);
	hWPT = WindowFromPoint(pt);

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);
	dwThreadIdCurr = GetCurrentThreadId();

	if(dwThreadIdWithPoint == dwThreadIdCurr)
	{
		if (hWDC == NULL || hWPT == hWDC
			|| IsParentOrSelf(hWPT, hWDC)
			|| IsParentOrSelf(hWDC, hWPT))
		{
			if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpString, cbString))
				&& (cbString > 0))
			{
				g_nTextAlign = GetTextAlign(hdc);
				g_nExtra     = GetTextCharacterExtra(hdc);
				GetCurrentPositionEx(hdc, &g_CurPos);
				GetTextMetrics(hdc, &g_tm);
				g_dwDCOrg.x = 0;
				g_dwDCOrg.y = 0;
				bRecAllRect = FALSE;
				GetStringRectW(hdc, lpString, cbString, nXStart,
					nYStart, &g_rcTotalRect, NULL);
				bRecAllRect = TRUE;

				if ((WindowFromDC != NULL)&&(WindowFromDC(hdc) == NULL))
				{
						g_dwDCOrg.x = 0;
						g_dwDCOrg.y = 0;

                    	// 01/19/2000
						// Fix Bug5: get word position error sometimes
						// Fix Bug5 begin

						AddToTextOutBufferW(hdc, lpString, cbString,
							nXStart, nYStart, NULL);

						//Fix Bug5 end
				}
				else
				{
						GetDCOrgEx(hdc, &g_dwDCOrg);

                    	// 01/19/2000
						// Fix Bug5: get word position error sometimes
						// Fix Bug5 begin
    
						GetCurMousePosWordW(hdc, lpString, cbString,
							nXStart, nYStart, NULL);

						//Fix Bug5 end
				}
			}
		}
	}

	// call TextOutW ���������ǵĺ���֮�󣬻ص�ϵͳ��TextOutW�������Ա�ϵͳ��������
	TextOutW(hdc, nXStart, nYStart, lpString, cbString);

	HookWin32Api(&g_TextOutWHook, HOOK_NEED_CHECK);

	return TRUE;
}

//Hook TextOutA�Ĺ���
DLLEXPORT BOOL WINAPI NHExtTextOutA(HDC hdc,
								    int X,	//��ǰ��Ҫ���Ƶ��ı�����ʼλ��
								    int Y,
								    UINT fuOptions,
								    CONST RECT *lprc,
								    LPCTSTR lpString,
								    UINT cbCount,
								    CONST INT *lpDx)
{
	POINT pt;
	HWND  hWDC;
	HWND  hWPT;
	DWORD dwThreadIdWithPoint = 0;
	DWORD dwThreadIdCurr = 0;

	// restore
	RestoreWin32Api(&g_ExtTextOutAHook, HOOK_NEED_CHECK);
/*
	if (cbCount != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHExtTextOutA : %s (%s) (%d)\n", "start", lpString, cbCount);
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("-> NHExtTextOutA : lpString(%s), cbCount(%d)\n", lpString, cbCount);

	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdc);
	hWPT = WindowFromPoint(pt);

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);
	dwThreadIdCurr = GetCurrentThreadId();

	if(dwThreadIdWithPoint == dwThreadIdCurr)
	{
		if (hWDC == NULL || hWPT == hWDC
			|| IsParentOrSelf(hWPT, hWDC)
			|| IsParentOrSelf(hWDC, hWPT))
		{
			if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpString, cbCount))
				&& (cbCount > 0))
			{
				g_nTextAlign = GetTextAlign(hdc);
				g_nExtra     = GetTextCharacterExtra(hdc);
				GetCurrentPositionEx(hdc, &g_CurPos);
				GetTextMetrics(hdc, &g_tm);
				g_dwDCOrg.x = 0;
				g_dwDCOrg.y = 0;
				bRecAllRect = FALSE;
				//findword.c�еĴ���,ȡ����ǰ�ַ��ľ��η�Χ
				GetStringRect(hdc, (LPSTR)lpString, cbCount, X, Y,
					&g_rcTotalRect, lpDx);
				bRecAllRect = TRUE;

				if ((WindowFromDC != NULL)&&(WindowFromDC(hdc) == NULL))
				{
						g_dwDCOrg.x = 0;
						g_dwDCOrg.y = 0;
                    
						AddToTextOutBuffer(hdc, (LPSTR)lpString, cbCount,
							X, Y, lpDx);
				}
				else
				{
						GetDCOrgEx(hdc, &g_dwDCOrg);
    
						//findword.c�еĴ���,ȡ����ǰλ���µĵ���
						GetCurMousePosWord(hdc, (LPSTR)lpString, cbCount,
							X, Y, lpDx);
				}
			}
		}
	}
	// call ExtTextOutA
	ExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);

	HookWin32Api(&g_ExtTextOutAHook, HOOK_NEED_CHECK);

	return TRUE;
}

//Hook TextOutW �Ĺ���
DLLEXPORT BOOL WINAPI NHExtTextOutW(HDC hdc,
								    int X,
								    int Y,
								    UINT fuOptions,
								    CONST RECT *lprc,
									LPCWSTR lpString,
								    UINT cbCount,
								    CONST INT *lpDx)
{
	POINT pt;
	HWND  hWDC;
	HWND  hWPT;

	DWORD dwThreadIdWithPoint = 0;
	DWORD dwThreadIdCurr = 0;

	// restore
	RestoreWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHExtTextOutW : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("-> NHExtTextOutW : lpString(%s), cbCount(%d)\n", lpString, cbCount);

	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdc);
	hWPT = WindowFromPoint(pt);

	// 01/17/2000
	// Fix Bug3: get word error when IE window overlaps.
	// Fix Bug3 begin

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);
	dwThreadIdCurr = GetCurrentThreadId();

	if(dwThreadIdWithPoint == dwThreadIdCurr)
	{
		// Fix Bug3 end

		if (hWDC == NULL || hWPT == hWDC
			|| IsParentOrSelf(hWPT, hWDC)
			|| IsParentOrSelf(hWDC, hWPT))
		{
			if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpString, cbCount))
				&& (cbCount > 0))
			{
	/*
				{
					//char cBuffer[0x100];
					//wsprintf(cBuffer, ">>>----> NHExtTextOutW : (%s) %d\n", lpTemp, cbCount);
					//OutputDebugString(cBuffer);					
				}
	*/
				g_nTextAlign = GetTextAlign(hdc);
				g_nExtra     = GetTextCharacterExtra(hdc);
				GetCurrentPositionEx(hdc, &g_CurPos);
				GetTextMetrics(hdc, &g_tm);
				g_dwDCOrg.x = 0;
				g_dwDCOrg.y = 0;
				bRecAllRect = FALSE;
				GetStringRectW(hdc, lpString, cbCount, X, Y,
					&g_rcTotalRect, lpDx);
				bRecAllRect = TRUE;

				//{DbgFilePrintf("--> NHExtTextOutW: lpTemp(%s)len(%d)\n", lpTemp, strlen(lpTemp));}
				//{DbgFilePrintf("--> NHExtTextOutW: X(%d)Y(%d), g_rcTotalRect(%d,%d,%d,%d)\n", X, Y, g_rcTotalRect.left, g_rcTotalRect.top, g_rcTotalRect.right, g_rcTotalRect.bottom);}

				if ((WindowFromDC != NULL)&&(WindowFromDC(hdc) == NULL))
				{
						g_dwDCOrg.x = 0;
						g_dwDCOrg.y = 0;

                    	// 01/19/2000
						// Fix Bug5: get word position error sometimes
						// Fix Bug5 begin

						AddToTextOutBufferW(hdc, lpString, 
							cbCount, X, Y, lpDx);

						// Fix Bug5 end
				}
				else
				{
						GetDCOrgEx(hdc, &g_dwDCOrg);

                    	// 01/19/2000
						// Fix Bug5: get word position error sometimes
						// Fix Bug5 begin
    
						GetCurMousePosWordW(hdc, lpString, 
							cbCount, X, Y, lpDx);

						// Fix Bug5 end
				}
			}
		}
	}

	// call ExtTextOutW
	ExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	
	//���÷�װ�õ�HooK����
	HookWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);

	return TRUE;
}
//�ڴ湲��
DLLEXPORT DWORD WINAPI BL_GetText32(LPSTR lpszCurWord, int nBufferSize, LPRECT lpWordRect)
{
	WORDRECT wr;
	DWORD dwCaretPlace;

	dwCaretPlace = BL_GetBuffer16(lpszCurWord, (short)nBufferSize, &wr);

	lpWordRect->left   = wr.left;   
	lpWordRect->right  = wr.right;  
	lpWordRect->top    = wr.top;    
	lpWordRect->bottom = wr.bottom;
/*
	{
		//char cBuffer[0x100];
		//wsprintf(cBuffer, "******BL_GetBuffer32 : (%s) %d %d %d %d\n", 
		//	lpszCurWord, wr.left, wr.top, wr.right, wr.bottom);
		//OutputDebugString(cBuffer);
	}
*/
	return dwCaretPlace;
}

//��BL_GetText32����
DLLEXPORT DWORD WINAPI BL_GetBuffer16(LPSTR lpszBuffer, short nBufferSize, LPWORDRECT lpWr)
{
        int len;
		char* pcFirstSpacePos = NULL;	//position of first space
		char* pcTemp = NULL;
		int nSrc = 0, nDest = 0;

        if (!g_bMouseInTotalWord)
        {
            g_szTotalWord[0] = 0x00;
            g_nCurCaretPlaceInTotalWord = -1;
	    }

        if ((len = strlen(g_szTotalWord)) >= nBufferSize)
        {
            len = nBufferSize - 1;
        }  

		while ((g_szTotalWord[len - 1] == ' ') && (len > 0))
		{
			len--;
			g_szTotalWord[len] = 0x00;
		}

		if (g_szTotalWord[0] < 0)
		{
			strncpy(lpszBuffer, g_szTotalWord, len);
			lpszBuffer[len] = 0x00;
			lpWr->left   = g_TotalWordRect.left;
			lpWr->right  = g_TotalWordRect.right;
			lpWr->top    = g_TotalWordRect.top;
			lpWr->bottom = g_TotalWordRect.bottom;
		}
		else
		{
			if (g_szTotalWord[0] == ' ')
			{
				//this conditions should not happen.
				strncpy(lpszBuffer, g_szTotalWord, len);
				lpszBuffer[len] = 0x00;
			}
			else
			{
				while (nSrc < len)
				{
					lpszBuffer[nDest] = g_szTotalWord[nSrc];
					nDest++;
					nSrc++;
					
					if (g_szTotalWord[nSrc]	== ' ' && nSrc < len)
					{
						lpszBuffer[nDest] = g_szTotalWord[nSrc];
						nDest++;
						nSrc++;
						while (g_szTotalWord[nSrc]	== ' ' && nSrc < len)
						{
							nSrc++;
						}
					}
				}
			}

			//strncpy(lpszBuffer, g_szTotalWord, len);
			lpszBuffer[len] = 0x00;
			lpWr->left   = g_rcFirstWordRect.left;
			lpWr->right  = g_rcFirstWordRect.right;
			lpWr->top    = g_rcFirstWordRect.top;
			lpWr->bottom = g_rcFirstWordRect.bottom;
		}

        return (DWORD)g_nCurCaretPlaceInTotalWord;    
}

//���� g_nGetWordStyle
DLLEXPORT DWORD WINAPI BL_SetGetWordStyle(int nGetWordStyle)
{
	g_nGetWordStyle = nGetWordStyle;

	if (nGetWordStyle == GETPHRASE_ENABLE)
	{
		g_nWordsInPhrase = -1;
		g_bPhraseNeeded = TRUE;
	}
	else
	{
		g_nWordsInPhrase = -1;
		g_bPhraseNeeded = FALSE;
	}
	return 0L;
}

//Suijun: �ж�hParent�Ƿ���hChild�� ���ȴ���
BOOL IsParentOrSelf(HWND hParent, HWND hChild)
{
	HWND hTemp = hChild;
	HWND hDesktop;
	
	if (hParent == NULL || hChild == NULL)
	{
		return FALSE;
	}

	hDesktop = GetDesktopWindow();
	while (hTemp != NULL && hTemp != hDesktop)
	{
		if (hTemp == hParent)
		{
			return TRUE;
		}

		hTemp = GetParent(hTemp);
	}

	return FALSE;
}
//Suijun: GetMessage�����Ļص�����
//Suijun: ����ҪHOOK��API��������HOOK 
//LJM: ��GetMessage������Hookԭ����
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	ProcessWin32API();
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

//Suijun: UINCODE�ַ�����ANSI��ת������
LPSTR UnicodeToAnsi(LPTSTR lpString, UINT cbCount)
{
	UINT i;

	if (*lpString > 0)
	{
		for (i = 0; i < cbCount; i++)
		{
			*(lpString + i) = *(lpString + 2 * i);
		}
		*(lpString + cbCount) = '\0';
	}

	return (LPSTR)lpString;
}

//Added by ZHHN on 2000.4

// Fix Bug1: NHW32 can not be free from memory when nhstart.exe exit
// Suijun: ����GetMessage hook��һ������ �û����̵���WindowsAPI���� GetMessageʱ��
// Suijun: ��DLLģ��ͻᱻ���ص��û�������ȥ��DllMain������ִ�У�
// Suijun: Ȼ��GetMsgProc�ص������ͻᱻ����*/
//LJM: ��Demo.exe��ʼ��ȡ�ʵ�ʱ��ᱻ����
DLLEXPORT BOOL WINAPI SetNHW32()
{
	if(g_hHook == NULL)
	{
		g_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstDll, 0);
		if (g_hHook == NULL)
		{
			//MessageBox(NULL, __TEXT("Error hooking."), 
			//		   __TEXT("GetWord"), MB_OK);
			return FALSE;
		}
	}

	return TRUE;
}
 
//Suijun: ж�ذ�װ��HOOK, ע���ʱ��DLL���ᱻ�û�����ж��
DLLEXPORT BOOL WINAPI ResetNHW32()
{
	if (g_hHook != NULL)
	{
		if (!UnhookWindowsHookEx(g_hHook))
		{
			return FALSE;
		}

		g_hHook = NULL;
	}

	return TRUE;
}

// Fix Bug1 end

//Added end