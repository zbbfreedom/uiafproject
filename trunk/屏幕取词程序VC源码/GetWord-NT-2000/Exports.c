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
// 定义输出、函数及程序中用到的结构体。
/////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "findword.h"
#include "exports.h"
#include "hookapi.h"
#include "public.h"
#include "dbgprint.h"

//Suijun: 设置为所有进程共享的数据段，每个进程装载该DLL的进程对数据
//Suijun: 的修改对其他进程也会发生影响
//LJM: Demo和其它进程都会加载NWH32.DLL,然后会共享使用这段数据,不会每次都分配这些数据.
#pragma data_seg(".sdata") //Start 共享数据段

UINT g_nFlag = 0;	//must share
HWND g_hNotifyWnd = NULL;	//must share
int g_MouseX = 0;	//must share
int g_MouseY = 0;	//must share
BOOL g_bNewGetWordFlag = FALSE;		//must share

char g_szTotalWord[BUFFERLENGTH] = "";	//must share
RECT g_TotalWordRect = {0,0,0,0};			// ノ癘魁Ч俱迭跋办must share
//当前鼠标下面是不是本次取到的所有词
int  g_bMouseInTotalWord = FALSE;           // ノ癘魁夹琌Ч俱迭いmust share
int  g_nCurCaretPlaceInTotalWord = -1;		// ノ癘魁夹Ч俱迭い竚must share
RECT g_rcFirstWordRect = {0,0,0,0};	//must share

int g_nGetWordStyle = 0;	//must share
int g_nWordsInPhrase = -1;	//must share
BOOL g_bPhraseNeeded = FALSE;	//must share

BOOL g_bHooked = FALSE;	//must share

//int g_nProcessHooked = 0;		//must share

char szMemDCWordBuff[BUFFERLENGTH] = "";	// ノ癘魁┮Τ MemDC い Text ゅセ
int  pnMemDCCharLeft[BUFFERLENGTH];			// ノ癘魁 TextOut い┮Τオ癸
int  pnMemDCCharRight[BUFFERLENGTH];		// ノ癘魁 TextOut い┮Τ癸
WORDPARA WordBuffer[MEMDC_MAXNUM];			// ノ癘魁 TextOut いち迭┮Τ迭獺
int nWordNum = 0;							// 癘魁 MemDC い虫迭计

#pragma data_seg()	//End 共享数据段申明结束

UINT g_uMsg = 0;

BOOL g_bOldGetWordFlag = FALSE;

HWND g_hWndParent = NULL;	//目标窗口祖先

BOOL  g_bAllowGetCurWord = FALSE;	//是否允许获取当前文本

int  g_CharType = CHAR_TYPE_OTHER;			// char类型（默认为不是ASCII，也不是汉字）

// 讽玡迭计沮挡篶( 讽玡迭パ块絯侥跋いち虫迭 )
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
HANDLE hMutex = NULL;		//线程同步信号

//声明GetMessage的回调函数
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
//UINCODE字符串到ANSI的转换函数
LPSTR UnicodeToAnsi(LPTSTR lpString, UINT cbCount);

//定义五个需要Hook API的数据结构 Start
//为什么要Hook这个当前不太清楚
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
//以上定义五个Hook Api的结构体 End

//LJM: 共享数据段的说明：对所有程序可读,可写
#pragma comment(linker,"-section:.sdata,rws")

////////////////////////////////////////////////////////////////////////////////
//
//	DllMain()
//  DLL 加载入口[该DLL会被Demo.exe和目标进程加载]
////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) 
	{
		case DLL_PROCESS_ATTACH:	//进程加载该DLL时
			
			g_hinstDll = hinstDLL;	//存储该进程的句柄放在g_hinstDll中[Demo.exe]
									//将五个Hook结构体中的hInst设为当前进程
			g_BitBltHook.hInst = hinstDLL;
			g_TextOutAHook.hInst = hinstDLL;
			g_TextOutWHook.hInst = hinstDLL;
			g_ExtTextOutAHook.hInst = hinstDLL;
			g_ExtTextOutWHook.hInst = hinstDLL;
			
			//注册为进程间通信注册windows消息-----------??不知道为什么要注册"Noble Hand"
			g_uMsg = RegisterWindowMessage("Noble Hand");
			if(!g_uMsg)
			{
				return FALSE;
			}

			//Added by ZHHN on 2000.2.2
			//Because forget to add this function before, with the result that it gets word
			//little slowly
			//注册为进程间通信注册windows消息-----------???
			BL_HASSTRING = RegisterWindowMessage(MSG_HASSTRINGNAME);
			if(!BL_HASSTRING)
			{
				return FALSE;
			}

			// create mutex
			//创建一个锁
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
		
		case DLL_PROCESS_DETACH:	//进程卸载该DLL时

			// restore 恢复
			NHUnHookWin32Api(); //Suijun: 撤销HOOK行动

			if (NULL != hMutex)
			{
				CloseHandle(hMutex);
			}

			break;
    }

    return TRUE;
}

//内联函数,相当于宏，提高性能,因为DLL一经加载，系统中的GetMessage都会过这里。
//GetMessage函数的回调函数 处理系统的GetMessage函数
__inline void ProcessWin32API()
{
	if (g_bNewGetWordFlag != g_bOldGetWordFlag)
	{
		if (g_bNewGetWordFlag)//是否调用了初始函数BL_SetFlag32设置为取词
		{
			WaitForSingleObject(hMutex, INFINITE); //Suijun: 等待异步信号，确保以下代码只有1个进程执行

			g_nGetWordStyle = g_nFlag;
			g_bAllowGetCurWord = TRUE;
			g_CurMousePos.x = g_MouseX;
			g_CurMousePos.y = g_MouseY;
			g_szCurWord[0] = 0x00;
			g_nCurCaretPlace = -1;

			if (!g_bHooked)	//是否已经Hook成功//第一次由于还没有Hook,会进入if循环体
			{
				NHHookWin32Api();	//没有Hook，加载Hook
				g_bHooked = TRUE;	//设置Hook成功标识
			}

			g_bAllowGetCurWord = TRUE;

			ReleaseMutex(hMutex);//释放独占锁
		}
		else
		{	//通过BL_SetFlag32设置g_bNewGetWordFlag=false 取消取词
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

//DLL 输出函数
//取词初始化(每次取词都需要设置)
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
	//获得包含指定点的窗口的句柄,这个也是后面需要Hook的窗口进程
	hWnd = WindowFromPoint(ptWindow);

	g_hWndParent = hWnd;

	switch (nFlag)
	{
//		case GETWORD_ENABLE:
		case GETWORD_D_ENABLE:
		case GETWORD_TW_ENABLE:
		case GETPHRASE_ENABLE:	//启用取词

			g_bNewGetWordFlag = TRUE;

			BL_SetGetWordStyle(GETPHRASE_ENABLE);

			dwCurrProcessId = GetCurrentProcessId();	//当前进程ID
			//获得当前取词目标进程ID
			GetWindowThreadProcessId(g_hWndParent, &dwProcessIdOfWnd);

			if(dwProcessIdOfWnd == dwCurrProcessId)	//如果当前Hook的进程和目标窗口进程一致,则开始处理
			{
				ProcessWin32API();	//手动执行Hook函数,处理取词
			}
			else
			{				
				SendMessage(g_hWndParent, g_uMsg, 0, 0);	//连着发送一个
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

//Hook 所有Win32Api
DLLEXPORT DWORD WINAPI NHHookWin32Api()
{
	//做一些取词前的清理工作(把之前取的数据清掉)
	g_nCurCaretPlace = -1;
	g_szCurWord[0] = 0x00;
	g_szTotalWord[0] = 0x00;
	g_nCurCaretPlaceInTotalWord = -1;
	g_CharType = CHAR_TYPE_OTHER;
	g_bMouseInTotalWord = FALSE;
	g_bAllowGetCurWord = TRUE;

	nWordNum = 0;
	szMemDCWordBuff[0] = 0x00;

	//开始Hook所有的文字输出函数
	HookAllTextOut();

	return BL_OK;
}

//调用UnHookAllTextOut() 卸载所有HOOK
DLLEXPORT DWORD WINAPI NHUnHookWin32Api()
{
	g_bAllowGetCurWord = FALSE;
	UnHookAllTextOut();
        
	return BL_OK;
}

//Suijun: 安装HOOK, HOOK  5个 Windows API函数BitBlt, TextOutA, TextOutW, ExtTextOutA, ExtTextOutW
void HookAllTextOut()
{
	HookWin32Api(&g_BitBltHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_TextOutAHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_TextOutWHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_ExtTextOutAHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_ExtTextOutWHook, HOOK_CAN_WRITE);
}

//Suijun: 卸载所有HOOK
void UnHookAllTextOut()
{
	RestoreWin32Api(&g_BitBltHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_TextOutAHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_TextOutWHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_ExtTextOutAHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);
}

//Hook在图片上画字的过程
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
		if (nWidth > 5)	//Xianfeng:大于5估计能放下字符
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

//Hook TextOutA的过程
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
	hWDC = WindowFromDC(hdc);	//Xianfeng:对方所在窗体句柄
	hWPT = WindowFromPoint(pt);	//Xianfeng:鼠标所在窗体句柄

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);	//Xianfeng:取得鼠标所在窗体线程ID
	dwThreadIdCurr = GetCurrentThreadId();		//Xianfeng:取得当前线程ID

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
					g_nTextAlign = GetTextAlign(hdc);			//Xianfeng:取得当前DC的文本对齐方式，在后面计算矩形用
					g_nExtra     = GetTextCharacterExtra(hdc);	//Xianfeng:取得字符间空隙，单位估计是像素
					GetCurrentPositionEx(hdc, &g_CurPos);		//Xianfeng:取得当前DC的逻辑位置
					GetTextMetrics(hdc, &g_tm);					//Xianfeng:取得当前DC的font的基本信息
        
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

							//Xianfeng:保存输出字符信息到内存
							AddToTextOutBuffer(hdc, (LPSTR)lpString, cbString,
											   nXStart, nYStart, NULL);
					}
					else
					{
							//Xianfeng:取回DC原点，通常的值是客户区左上角相对于窗体左上角的偏移量
							GetDCOrgEx(hdc, &g_dwDCOrg);	
        
							//Xianfeng:获取当前鼠标下的字
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

//Hook TextOutW 的过程
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

	// call TextOutW 处理完我们的函数之后，回调系统的TextOutW函数，以便系统继续进行
	TextOutW(hdc, nXStart, nYStart, lpString, cbString);

	HookWin32Api(&g_TextOutWHook, HOOK_NEED_CHECK);

	return TRUE;
}

//Hook TextOutA的过程
DLLEXPORT BOOL WINAPI NHExtTextOutA(HDC hdc,
								    int X,	//当前需要绘制的文本的起始位置
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
				//findword.c中的代码,取出当前字符的矩形范围
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
    
						//findword.c中的代码,取出当前位置下的单词
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

//Hook TextOutW 的过程
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
	
	//调用封装好的HooK过程
	HookWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);

	return TRUE;
}
//内存共享
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

//被BL_GetText32调用
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

//设置 g_nGetWordStyle
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

//Suijun: 判断hParent是否是hChild的 祖先窗口
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
//Suijun: GetMessage函数的回调函数
//Suijun: 对需要HOOK的API函数进行HOOK 
//LJM: 对GetMessage函数的Hook原函数
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	ProcessWin32API();
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

//Suijun: UINCODE字符串到ANSI的转换函数
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
// Suijun: 设置GetMessage hook，一旦发现 用户进程调用WindowsAPI函数 GetMessage时候，
// Suijun: 该DLL模块就会被加载到用户进程中去，DllMain函数先执行，
// Suijun: 然后GetMsgProc回调函数就会被调用*/
//LJM: 在Demo.exe初始化取词的时候会被加载
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
 
//Suijun: 卸载安装的HOOK, 注意此时该DLL将会被用户进程卸载
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