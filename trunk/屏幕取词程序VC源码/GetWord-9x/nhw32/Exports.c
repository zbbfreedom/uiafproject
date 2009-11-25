
#include <windows.h>
#include "NHTW32.h"
#include "import.h"
#include "HookApi.h"

#include "FindWord.h"

#include "..\pub3216.h"

#include "dbgfunc.h"

#pragma data_seg(".sdata")  //start of section
BOOL bDebugBegin = TRUE;
HANDLE hMutex = NULL;                // 信号量，用於互斥。
HANDLE hWork = NULL;
APIHOOKSTRUCT g_ExtTextOutWHook = {
									"Gdi32.dll",		// The module name that the api in it.
									"ExtTextOutW",		// The api name that it will be hooked.
									10,
									NULL,				// This api point.
									{0, 0, 0, 0, 0, 0, 0},	// Use to record five byte at begin of api.
														// these data is get from this program. 
														// the system may be already run the other 
														// program that these program change api too.

									NULL,				// The module name that the hook api in it.
									"NhExtTextOutW",	// The hook api name that it will replace the api.
									NULL,				// This hook api point.
									{0, 0, 0, 0, 0, 0, 0},	// Use to record five byte at begin of api.
														// these data is get from this program. 
														// the system may be already run the other 
														// program that these program change api too.
									0,
														// Use to record five byte at begin of api.
														// these data is get from WINICE. it is pure.
														// Used when the BLIEA restart when BLIEA abort,
														// Use to restore this api data.
									{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
								  };

// 当前词数据结构·( 当前词：为由输入缓冲区中切出的单词 )
char g_szCurWord[WORDMAXLEN] = "";	// 当前词缓冲区·
RECT g_CurWordRect;					// 记录当前词区域·
int  g_nCurCaretPlace;				// 记录当前光标位置·
// 当前词数据结构·

BOOL  g_bAllowGetCurWord = FALSE;	//
HWND  g_hNotifyWnd = NULL;			//
POINT g_CurMousePos;				// 记录当前光标位置·

UINT         g_nTextAlign;
POINT        g_dwDCOrg;
int          g_nExtra;
POINT        g_CurPos;
TEXTMETRIC   g_tm;

///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// 用於修改在计算 TA_CENTER 情况的失误。
BOOL bRecAllRect = TRUE;
RECT g_rcTotalRect;
// End Modify
///////////////////////////////////////////////////////////////////////////

#ifdef _DICTING_
/////////////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/19
// 主要解决通配符的问题。
int g_nGetWordStyle;
// End Add.
/////////////////////////////////////////////////////////////////////////////////
#endif

extern BOOL g_bCanWrite;

BOOL g_bHooked = FALSE;
#pragma data_seg()

//*********************************************************************
//Added by XGL, Jan 7th, 1999
//for get word in Acrobat Reader
extern HANDLE g_heventGetWord ;	//event handle

extern HANDLE g_hMappedFile ;	//handle of file-mapping object

extern void *g_pMemFile ;		//pointer to memfile

BOOL g_bAcrobatReaderWord = FALSE ;	//indicate that the word was got
									//from Acrobat Reader

//end of addition
//*********************************************************************

typedef BOOL (WINAPI *EXTTEXTOUTW_PROC)(HDC, int, int, UINT, CONST RECT *, 	LPCTSTR, UINT, CONST INT *);

DLLEXPORT DWORD WINAPI BL_SetFlag32(UINT nFlag, HWND hNotifyWnd, int MouseX, int MouseY)
{
	//Modified by XGL, Jan 7th, 1999
	//for Acrobat Reader
	POINT ptWindow ;
	HWND hWnd ;
	void *pTemp ;

	ptWindow.x = MouseX ;
	ptWindow.y = MouseY ;
	hWnd = WindowFromPoint(ptWindow) ;

	//Are we getting word from Acrobat Reader
	__try
	{
		if (NULL != g_pMemFile 
			&& EXISTED == *((BYTE*)g_pMemFile + PLUGIN_FLAG_POS)
			&& hWnd == *((HWND*)((BYTE*)g_pMemFile + WND_HANDLE_POS)))
		{
			if(GETPHRASE_ENABLE == nFlag ||
				GETWORD_TW_ENABLE == nFlag ||
				GETWORD_D_ENABLE == nFlag ||
				GETWORD_D_TYPING_ENABLE == nFlag)
			{
				//fill mouse pos buffer
				pTemp = (BYTE*)g_pMemFile + MOUSE_X_POS ;
				*((int*)pTemp) = MouseX ;
				pTemp = (BYTE*)g_pMemFile + MOUSE_Y_POS ;
				*((int*)pTemp) = MouseY ;
				pTemp = (BYTE*)g_pMemFile + WND_NOTIFIY_POS ;
				*((HWND*)pTemp) = hNotifyWnd ;
				
				*((BYTE*)g_pMemFile + WORD_POS) = 0x0 ;

				//set get word flag
				ResetEvent(g_heventGetWord) ;
				return BL_OK;
			}
			else
			{
				//GETWORD_DISABLE and so on
				return BL_OK ;
			}
		}
	}
	__except (1)
	{
		SetEvent(g_heventGetWord) ;
	}
	//end of modification, XGL, Jan 7th, 1999

	switch (nFlag)
	{
#ifdef _DICTING_
/////////////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/19
// 主要解决通配符的问题。
		// 打字／用字取词。
		case GETPHRASE_ENABLE:
		case GETWORD_TW_ENABLE:
		// 查字取词。
		case GETWORD_D_ENABLE:
			// 设置取词方式。
			WaitForSingleObject(hMutex, INFINITE);
			g_nGetWordStyle = nFlag;

			BL_SetGetWordStyle(GETPHRASE_ENABLE/*nFlag*/);
			BL_SetPara16((short)hNotifyWnd, (short)MouseX, (short)MouseY);

			g_bAllowGetCurWord = TRUE;
			g_hNotifyWnd = hNotifyWnd;
			g_CurMousePos.x = MouseX;
			g_CurMousePos.y = MouseY;
	        g_szCurWord[0] = 0x00;
		    g_nCurCaretPlace = -1;

			BL_HookWinApi16();

			if (!g_bHooked)
			{
				HookWin32Api(&g_ExtTextOutWHook, HOOK_CAN_WRITE);
				g_bHooked = TRUE;
			}

			g_bAllowGetCurWord = TRUE;

			ReleaseMutex(hMutex);

			break;
// End Modify

		//Added by XGL, Nov 18th, 1998
		//for typing assistant specially, it don't need phrase
		case GETWORD_D_TYPING_ENABLE:
			// 设置取词方式。
			WaitForSingleObject(hMutex, INFINITE);
			g_nGetWordStyle = nFlag;
			BL_SetGetWordStyle(GETWORD_D_ENABLE);
			BL_SetPara16((short)hNotifyWnd, (short)MouseX, (short)MouseY);

			g_bAllowGetCurWord = TRUE;
			g_hNotifyWnd = hNotifyWnd;
			g_CurMousePos.x = MouseX;
			g_CurMousePos.y = MouseY;
	        g_szCurWord[0] = 0x00;
		    g_nCurCaretPlace = -1;

			BL_HookWinApi16();

			if (!g_bHooked)
			{
				HookWin32Api(&g_ExtTextOutWHook, HOOK_CAN_WRITE);
				g_bHooked = TRUE;
			}

			g_bAllowGetCurWord = TRUE;

			ReleaseMutex(hMutex);

			break;
		//Adding ends. XGL, Nov 18th, 1998
////////////////////////////////////////////////////////////////////////////////
#else
		case GETWORD_ENABLE:
			WaitForSingleObject(hMutex, INFINITE);
			BL_SetPara16((short)hNotifyWnd, (short)MouseX, (short)MouseY);

			g_bAllowGetCurWord = TRUE;
			g_hNotifyWnd = hNotifyWnd;
			g_CurMousePos.x = MouseX;
			g_CurMousePos.y = MouseY;
	        g_szCurWord[0] = 0x00;
		    g_nCurCaretPlace = -1;

			BL_HookWinApi16();

			if (!g_bHooked)
			{
				HookWin32Api(&g_ExtTextOutWHook, HOOK_CAN_WRITE);
				g_bHooked = TRUE;
			}
			g_bAllowGetCurWord = TRUE;
			ReleaseMutex(hMutex);

			break;
#endif
		case GETPHRASE_DISABLE:
		case GETWORD_DISABLE:
		case GETWORD_D_TYPING_DISABLE:
			WaitForSingleObject(hMutex, INFINITE);
			
			g_bAllowGetCurWord = FALSE;
			if (g_bHooked)
			{
				RestoreWin32Api(&g_ExtTextOutWHook, HOOK_ONLY_READ);
				g_bHooked = FALSE;
			}
			BL_UnHookWinApi16();

			ReleaseMutex(hMutex);
			break;
	}
	return BL_OK;
}

DLLEXPORT DWORD WINAPI BL_GetText32(LPSTR lpszCurWord, int nBufferSize, LPRECT lpWordRect)
{
	WORDRECT wr;
	DWORD dwCaretPlace;

	//DbgPrintfToICE("Nhw32.dll   GetText: Begin.");
	//modified by XGL, Jan 7th, 1999
	//for Acrobat Reader
	void *pTemp ;
	int nStrLen ;
	LPSTR lpstrLetter;

	if (WAIT_TIMEOUT == WaitForSingleObject(g_heventGetWord, NH_WAIT_TIME))
	{
		SetEvent(g_heventGetWord) ;
	}
	
	__try
	{
		//check if we got word from Acrobat Reader
		if (NULL != g_pMemFile
			&& *((BYTE*)g_pMemFile + WORD_POS) != 0x0)
		{
			pTemp = (BYTE*)g_pMemFile + WORD_POS ;
			nStrLen = strlen(pTemp) ;

			//get proper len for copy
			nStrLen = (nStrLen < nBufferSize) ? nStrLen : nBufferSize ;

			strncpy(lpszCurWord, (LPSTR)pTemp, nStrLen) ;
			lpszCurWord[nStrLen] = 0x0 ;

			//copy word rect
			pTemp = (BYTE*)g_pMemFile + RECT_POS ;
			lpWordRect->left   = *((LONG*)pTemp) ;   
			pTemp = (BYTE*)pTemp + sizeof(LONG) ;
			lpWordRect->top    = *((LONG*)pTemp) ;    
			pTemp = (BYTE*)pTemp + sizeof(LONG) ;
			lpWordRect->right  = *((LONG*)pTemp) ;  
			pTemp = (BYTE*)pTemp + sizeof(LONG) ;
			lpWordRect->bottom = *((LONG*)pTemp) ;

			//set buffer to empty
			*((BYTE*)g_pMemFile + WORD_POS) = 0x0 ;
		}
		else
		{
			dwCaretPlace = BL_GetBuffer16(lpszCurWord, (short)nBufferSize, &wr);
			lpWordRect->left   = wr.left;   
			lpWordRect->right  = wr.right;  
			lpWordRect->top    = wr.top;    
			lpWordRect->bottom = wr.bottom;
		}
	}
	__except (1)
	{
	}
	//end of modification
	return dwCaretPlace;
}

DLLEXPORT BOOL WINAPI NhExtTextOutW(HDC hdc,			// handle to device context 
									int X,				// x-coordinate of reference point 
									int Y,				// y-coordinate of reference point 
									UINT fuOptions,		// text-output options 
									CONST RECT *lprc,	// optional clipping and/or opaquing rectangle 
									LPCTSTR lpString,	// points to string 
									UINT cbCount,		// number of characters in string 
									CONST INT *lpDx 	// pointer to array of intercharacter spacing values  
								   )
{
	EXTTEXTOUTW_PROC ExtTextOutWApi;
	EXTTEXTOUTW_PROC ExtTextOutAApi;
	char szTemp[256];
	int  nTempLen;
	BOOL bWOrA;
	HANDLE hOldMutex;

//////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1998.02.19
// Reason: 
//        Because the CX register in Chinese version Windows different from English version.
	WORD wCXRegister, wCXInWCode, wCXInACode;
	LPWORD temp;

	_asm
	{
		mov wCXRegister, cx;
	};

	hOldMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEXNAME);

	WaitForSingleObject(hOldMutex, INFINITE);

	ExtTextOutWApi = (EXTTEXTOUTW_PROC)((DWORD)g_ExtTextOutWHook.lpWinApiProc - 10);
	ExtTextOutAApi = (EXTTEXTOUTW_PROC)((DWORD)g_ExtTextOutWHook.lpWinApiProc - 4);
	
	temp = (LPWORD)ExtTextOutWApi;
	wCXInWCode = temp[1];
	temp = (LPWORD)ExtTextOutAApi;
	wCXInACode = temp[1];
	if (wCXRegister == wCXInACode)
	{	// ExtTextOutA
		bWOrA = 1;
	}
	else
	{	// ExtTextOutW
		bWOrA = 0;
	}
// Modify end.
//////////////////////////////////////////////////////////////////////////


//	_asm				// 用於判断当前是否为UNICODE.
//	{
//		cmp cx, 20b3h;
//		je  exttextouta;
//		mov bWOrA, 0;	// ExtTextOutW
//		jmp end;
//exttextouta:
//		mov bWOrA, 1;	// ExtTextOutA
//end:
//	};
// End modify
//////////////////////////////////////////////////////////////////////////


	// restore the old textout.
#ifdef _DEUBG
		DbgPrintfToICE("Nhw32.dll   ExtTextOutW: Restore Begin.");
#endif
	RestoreWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);
#ifdef _DEUBG
		DbgPrintfToICE("Nhw32.dll   ExtTextOutW: Restore End.");
#endif

	// call old textout.
	if (bWOrA == 0)
	{
		ExtTextOutWApi(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
		nTempLen = WideCharToMultiByte(CP_ACP, 0, (const unsigned short *)lpString, cbCount, szTemp, 256, NULL, NULL);
		szTemp[nTempLen] = 0x00;

///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// 用於修改在计算 TA_CENTER 情况的失误。
		bRecAllRect = FALSE;
		g_dwDCOrg.x = 0;
		g_dwDCOrg.y = 0;
		GetStringRect(hdc, (LPSTR)szTemp, nTempLen, X, Y, &g_rcTotalRect, (LPINT)lpDx);
		bRecAllRect = TRUE;
// End Modify
///////////////////////////////////////////////////////////////////////////

		BL_ExtTextOutW(hdc, X, Y, fuOptions, (LPRECT)lprc, (LPSTR)szTemp, nTempLen, (LPINT)lpDx);
	}
	else
	{
		ExtTextOutAApi(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	if (g_bHooked)
	{
		HookWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);
	}

	ReleaseMutex(hOldMutex);
	CloseHandle(hOldMutex);

	return (TRUE);
}

DWORD BL_ExtTextOutW(HDC hDC,
					 int X,
					 int Y,
					 UINT fuOptions,
					 LPRECT lprc,
					 LPSTR lpString,
					 UINT cbCount,
					 LPINT lpDx)
{
	//Added by XGL, Nov 3rd, 1998
	//We cannot get corret word with Explorer as background window
	//in Windows98, so this situation must be dealt with.
	if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpString, cbCount)) && (cbCount > 0))
	{
		g_nTextAlign = GetTextAlign(hDC);
		g_nExtra     = GetTextCharacterExtra(hDC);
		GetCurrentPositionEx(hDC, &g_CurPos);
		GetTextMetrics(hDC, &g_tm);

		if (WindowFromDC(hDC) == NULL)
		{
			// 赋零，用於避免MEMDC对串位置的影响·
			g_dwDCOrg.x = 0;
			g_dwDCOrg.y = 0;

			AddToTextOutBuffer(hDC, (LPSTR)lpString, cbCount, X, Y, lpDx);
		}
		else
		{
			GetDCOrgEx(hDC, &g_dwDCOrg);

			GetCurMousePosWord(hDC, (LPSTR)lpString, cbCount, X, Y, lpDx);
		}
	}
	else
	{
	}
	return (TRUE);
}
