//**************************************************************
// SrhWord.C 
//
// Copyright 1997 by ITC, PCB. All rights reserved.
//
// Author:        FengShuen Lu / Gang Fang / Gang Yan
//
// Date:          1997.5.30
//
// Description:   ����ڴ��֣�������ȡ��ָ��λ�ô��ĵ��ʣ������ع��λ��.
//              
// Function:   
//                LibMain()       : �ú����� DLL �� LOAD ʱ���Զ����ã���춳�ʼ�����ݡ�
//                DllEntryPoint() : �ú����� DLL �� LOAD ʱ���Զ����ã���춳�ʼ��Thunk���̡�
//                WEP()           : �ú����� DLL ���ͷ�ʱ���Զ����á�
//
// Notes:
//
// Update:
//
//  Date       Name         Description
// =======    ======       ====================================
// 97.5.30    ��  ��        version 1.0.
//   
// 97.11.13   ��  ��        ��������������ֺϲ���
//                          ��Ҫ�������ʵ�֣���Ϊ���� API �ᣬ�� IE4 �� UNICODE 
//                          �Ĵ��벻��Ҫ���⴦��ͬʱ��Ҫ������/����������дʵ�
//                          �������ͳһ��
//                          ��Ҫ�Ķ�ʦע��һ����Ϣ������� FINDWORD.C ���ڵõ���
//                          ���������������Ϣ���Խ�����ʾ��
//
//                          ��춲����������İ汾���Ƿ��������������������
//                          �����йصĴ���������������룬�� _DICTING_ ����ơ�
//
// 97.11.18   ��  ��        �޸Ķ� TA_CENTER ��ʽ���������BUG.
//
//**************************************************************

#include <windows.h>
#include <string.h>
#include <assert.h>
#include "srhword.h"
#include "findword.h"
#include "hookapi.h"
#include "..\pub3216.h"

#ifdef _DEBUG
#include "DbgFunc.h"
#endif //_DEBUG

// ���� Thunk �� 16 λ�� 32 λ DLL ����
#define DLL32NAME "NHW32.DLL"
#define DLL16NAME "NHW16.DLL"

// API ���ؽṹ��

// Gdi!TextOut
APIHOOKSTRUCT g_TextOutHook = {
								"GDI",					// The module name that the api in it.
								"TextOut",				// The api name that it will be hooked.
								0,
								NULL,					// This api point.
								{0, 0, 0, 0, 0},		// Use to record five byte at begin of api.
														// these data is get from this program. 
														// the system may be already run the other 
														// program that these program change api too.

								NULL,					// The module name that the hook api in it.
								"BlTextOut",			// The hook api name that it will replace the api.
								NULL,					// This hook api point.
								{0, 0, 0, 0, 0},		// Use to record five byte at begin of api.
														// these data is get from this program. 
														// the system may be already run the other 
														// program that these program change api too.
								0,
														// Use to record five byte at begin of api.
														// these data is get from WINICE. it is pure.
														// Used when the BLIEA restart when BLIEA abort,
														// Use to restore this api data.
								{0X55, 0X8B, 0XEC, 0X68, 0XB7}
							  };

// Gdi!ExtTextOut
APIHOOKSTRUCT g_ExtTextOutHook = {
									"GDI",
									"ExtTextOut",
									0,
									NULL,
									{0, 0, 0, 0, 0},
									NULL,
									"BlExtTextOut",
									NULL,
									{0, 0, 0, 0, 0},
									0,
									{0xb8, 0xe7, 0x05, 0x45, 0x55}
								 };

// Gdi!BitBlt
// ����й� Text ������ǲ������� MemDC ��������ڲ��� BitBlt �������Ļ��
// �����Ҫ���� BitBlt ���õ���������Ļ������ DC ���й���Ϣ��
APIHOOKSTRUCT g_BitBltHook = {
								"GDI",
								"BitBlt",
								0,
								NULL,
								{0, 0, 0, 0, 0},
								NULL,
								"BLBitBlt",
								NULL,
								{0, 0, 0, 0, 0},
								0,
								{0x55, 0x8b, 0xec, 0xb8, 0xf2}
							 };

// �ú����� 16 λϵͳ�����δ����������
WINDOWFROMDC_PROC WindowFromDC = NULL;

// ���������ݽṹ��
int  g_nCurrWindowVer = CHINESE_PRC;				// ��춼�¼��ǰ�������࣬��Ҫ��춷ֱ�������ĺͷ������ġ�
													// ��ʱ���á�
char g_szTotalWord[BUFFERLENGTH] = "";				// �����ʼ�¼��������
													// ��춼�¼�ּ�������ĵ��ʡ�
RECT g_TotalWordRect;								// ��춼�¼�����ʵ������С��
int  g_CharType = CHAR_TYPE_OTHER;					// ��춼�¼�����ʵ����͡�
													// �ڳ�������Ҫ����б��¼���Ĵ��������Ƿ������еĵ���������ͬ��
int  g_nCurCaretPlaceInTotalWord = -1;				// ��춼�¼������������е�λ�á�
int  g_bMouseInTotalWord = FALSE;					// ��춼�¼����Ƿ����������С�
													// �ڳ����иñ�ʾ��Ҫ����ж��Ƿ���Ҫ���buffer.
// ���������ݽṹ��

// ��ǰ�����ݽṹ��( ��ǰ�ʣ�Ϊ�����뻺�������г��ĵ��� )
char g_szCurWord[WORDMAXLEN] = "";					// ��ǰ�ʻ�������
RECT g_CurWordRect;									// ��¼��ǰ������
int  g_nCurCaretPlace;								// ��¼��ǰ���λ�á�
// ��ǰ�����ݽṹ��

// ��¼�� MemDC �йص���Ϣ��
// ����й� Text ������ǲ������� MemDC ��������ڲ��� BitBlt �������Ļ��
// �����Ҫ�� Text ��������Ϣȫ����¼��
char szMemDCWordBuff[BUFFERLENGTH] = "";			// ��춼�¼���� MemDC �е� Text �ı���
int  pnMemDCCharLeft[BUFFERLENGTH];					// ��춼�¼�� TextOut �������ֵ������ֵ��
int  pnMemDCCharRight[BUFFERLENGTH];				// ��춼�¼�� TextOut �������ֵ������ֵ��
WORDPARA WordBuffer[MEMDC_MAXNUM];					// ��춼�¼�� TextOut ���д������дʵ���Ϣ��
int nWordNum = 0;									// ��¼ MemDC �е��ʵĸ�����
// ��¼�� MemDC �йص���Ϣ��

BOOL  g_bAllowGetCurWord = FALSE;					// �Ƿ�ȡ���жϱ�־���� UnHook û�гɹ�ʱ�����á�
HWND  g_hNotifyWnd = NULL;							// ��¼ȡ����Ҫ֪ͨ�Ĵ��ھ����
POINT g_CurMousePos;								// ��¼��ǰ���λ�á�

UINT         g_nTextAlign;
DWORD        g_dwDCOrg;
int          g_nExtra;
POINT        g_CurPos;
TEXTMETRIC   g_tm;
///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// ����޸��ڼ��� TA_CENTER �����ʧ��
BOOL bRecAllRect = FALSE;
RECT g_rcTotalRect;
// End Modify
///////////////////////////////////////////////////////////////////////////
//Added by XGL,Sep 29th,1998
RECT g_rcFirstWordRect;								// contains the rect of first word of phrase
//Adding ends.
///////////////////////////////////////////////////////////////////////////

#ifdef _DICTING_
///////////////////////////////////////////////////////////////////////////
// Add by Yan/Gang   11/13/1997
// ������������֪ͨȡ������Ҫ�ĵ��ʡ�
UINT BL_HASSTRING;
// Add end.
///////////////////////////////////////////////////////////////////////////
#endif

#ifdef _DICTING_
/////////////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/19
// ��Ҫ���ͨ��������⡣
int g_nGetWordStyle;
// End Add.
/////////////////////////////////////////////////////////////////////////////////

//Added by XGL, Sep 17th, 1998
int g_nWordsInPhrase = -1 ;	//current word number in phrase
BOOL g_bPhraseNeeded = FALSE ; //GETPHRASE_D(or TW)_ENABLE defined
int g_nPhraseCharType = CHAR_TYPE_OTHER ;
//Adding ends

#endif

BOOL FAR PASCAL __export DllEntryPoint (DWORD dwReason,
										WORD  hInst,
										WORD  wDS,
										WORD  wHeapSize,
										DWORD dwReserved1,
										WORD  wReserved2);

BOOL FAR PASCAL thk_ThunkConnect16(LPSTR pszDll16,
								   LPSTR pszDll32,
								   WORD  hInst,
								   DWORD dwReason);

HANDLE ghDLLInst;


//*********************************************************************
// Name :          LibMain()
//
// Descriptions:   �ú����� DLL �� LOAD ʱ���Զ����ã���춳�ʼ�����ݡ�
//
// Arguments :     HINSTANCE   hinst       : Identifies the instance of the DLL. 
//                 WORD        wDataSeg    : Specifies the value of the data segment 
//                                           (DS) register. 
//                 WORD        cbHeapSize  : Specifies the size of the heap defined
//                                           in the module-definition file. 
//                                           (The LibEntry routine in LIBENTRY.OBJ 
//                                           uses this value to initialize the local heap.) 
//                 LPSTR       lpszCmdLine : Points to a null-terminated string specifying 
//                                           command-line information. This parameter is 
//                                           rarely used by DLLs. 
//
// Return :        int   : The function should return 1 if it is successful. Otherwise, it should return 0.
//
// Notes :
//
// Update:
//
//  Date       Name         Description
// =======    ======       ====================================
// 97.5.30    ��  ��        version 1.0.
//   
// 97.11.13   ��  ��        ����һ��ע����Ϣ��
//
//*********************************************************************
int CALLBACK LibMain(HINSTANCE hInst,
					 WORD wDataSeg,
					 WORD cbHeapSize,
					 LPSTR lpszCmdLine)
{
	HMODULE hUser16;
                
	// �õ� WindowFromDC �����ľ����
	hUser16 = LoadLibrary("USER.EXE");
	if (hUser16 != NULL)
	{
		WindowFromDC = (WINDOWFROMDC_PROC)GetProcAddress(hUser16, "WINDOWFROMDC");
	}
	else
	{
		WindowFromDC = NULL;
	}

	// ��ʼ�����ݡ�
	ghDLLInst = hInst;
	g_nCurCaretPlace = -1;
	g_CurMousePos.x  = 0;
	g_CurMousePos.y  = 0;
	g_TextOutHook.hInst    = hInst;
	g_ExtTextOutHook.hInst = hInst;
	g_BitBltHook.hInst     = hInst;

#ifdef _DICTING_
	///////////////////////////////////////////////////////////////////////////
	// Add by Yan/Gang   11/13/1997
	// ������������֪ͨȡ������Ҫ�ĵ��ʡ�
	BL_HASSTRING = RegisterWindowMessage(MSG_HASSTRINGNAME);
	// Add end.
	///////////////////////////////////////////////////////////////////////////
#endif

//	HookAllTextOut();
	return (int)1;
}                                        

//*********************************************************************
// Name :          DllEntryPoint()
//
// Descriptions:   �ú����� DLL �� LOAD ʱ���Զ����ã���춳�ʼ�� Thunk ���̡�
//
// Notes :
//
// Update:
//
//*********************************************************************
BOOL FAR PASCAL __export DllEntryPoint (DWORD dwReason,
										WORD  hInst,
										WORD  wDS,
										WORD  wHeapSize,
										DWORD dwReserved1,
										WORD  wReserved2)
{
	// ��ʼ�� Thunk ���̡�
	if (!thk_ThunkConnect16(DLL16NAME,
							DLL32NAME,
							hInst,
							dwReason))
	{
		return FALSE;
	}

	return TRUE;
}

//*********************************************************************
// Name :          WEP()
//
// Descriptions:   �ú����� DLL ���ͷ�ʱ���Զ����á�
//
// Arguments :     int nExitType : Specifies whether all of Windows is shutting 
//                                 down or only the individual library. This 
//                                 parameter can be either WEP_FREE_DLL or 
//                                 WEP_SYSTEM_EXIT. 
//
// Return :        int   : The function should return 1 if it is successful. Otherwise, it should return 0.
//
//  Date       Name         Description
// =======    ======       ====================================
// 97.5.30    ��  ��        version 1.0.
//
//*********************************************************************
int CALLBACK WEP(int nExitType)
{
	int i = 16 ;
        UnHookAllTextOut();    
#ifdef _DEBUG        
		DbgPrintf("This is %d bit dll", i) ;
#endif		
        return 1;
}

// Use to set current language sytle.
// Because file version functions do not work well in 16bit file.
// So I get language version in 32bit file. and then use thunk to send item 
// to 16 bit.
DWORD FAR PASCAL _export BL_SetVer16(int nLangVer)
{
        g_nCurrWindowVer = nLangVer;
        return BL_OK;
}

/////////////////////////////////////////////////////////////////////////////////
// Add by Yan/Gang 1997/11/19
// ��Ҫ���ͨ��������⡣
DWORD FAR PASCAL _export BL_SetGetWordStyle(int nGetWordStyle)
{
#ifdef _DICTING_
	g_nGetWordStyle = nGetWordStyle;

	//Added by XGL, Sep 17th,1997
	//if phrase was required
	if (nGetWordStyle == GETPHRASE_ENABLE)
	{
		g_nWordsInPhrase = -1 ;
		g_bPhraseNeeded = TRUE ;
	}
	else
	{
		g_nWordsInPhrase = -1 ;
		g_bPhraseNeeded = FALSE ;
	}
	//Adding ends.
#endif
	return 0L;
}
// End Add.
/////////////////////////////////////////////////////////////////////////////////

// Use to record some parament. include wnd rect. and reset buffer.    
DWORD FAR PASCAL _export BL_SetPara16(short hNotifyWnd, short MouseX, short MouseY)
{
        g_hNotifyWnd    = hNotifyWnd;
        g_CurMousePos.x = MouseX;
        g_CurMousePos.y = MouseY;

        return BL_OK;
}

DWORD FAR PASCAL _export BL_GetBuffer16(LPSTR lpszBuffer, short nBufferSize, LPWORDRECT lpWr)
{
        int len;
		char* pcFirstSpacePos = NULL ;	//position of first space
		char* pcTemp = NULL ;
		int nSrc = 0, nDest = 0 ;
    
        if (!g_bMouseInTotalWord)
        {
#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL          GetBuffer: Caret not in word, clear buffer.");
#endif
                g_szTotalWord[0] = 0x00;
                g_nCurCaretPlaceInTotalWord = -1;
    }

        if ((len = strlen(g_szTotalWord)) >= nBufferSize)
        {
                len = nBufferSize - 1;
        }  

		//Modified by XGL, Oct 28th, 1998
		//In IE4, cannot get correct Chinese word,
		//so we have to use former version for Chinese word
/*		//Added by XGL, Sep 29th, 1998
		if (g_nPhraseCharType == CHAR_TYPE_HZ)
		{
			//get the word before the first space
			if (pcFirstSpacePos = strchr(g_szTotalWord, ' '))
			{
				len = pcFirstSpacePos - g_szTotalWord ;
			}
		}
*/
		//remove tail space(s)
		while ((g_szTotalWord[len - 1] == ' ') && (len > 0))
		{
			len-- ;
			g_szTotalWord[len] == 0x00 ;
		}
		//Adding ends.

		//Modified by xgl, Oct 26th, 1998
		//for English, we combine spaces between words to one
		//we do it by copying chars one by one
		//strncpy will no longer be used
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
					lpszBuffer[nDest] = g_szTotalWord[nSrc] ;
					nDest++ ;
					nSrc++ ;
					
					if (g_szTotalWord[nSrc]	== ' ' && nSrc < len)
					{
						lpszBuffer[nDest] = g_szTotalWord[nSrc] ;
						nDest++ ;
						nSrc++ ;
						while (g_szTotalWord[nSrc]	== ' ' && nSrc < len)
						{
							nSrc++ ;
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
        //Modification ends. xgl, Oct 26th, 1998

#ifdef _DEBUG
//        DbgPrintf("NhWSrh.DLL          g_szCurWord: %s", g_szCurWord);
//        DbgPrintf("NhWSrh.DLL          Rect l: %d, r: %d, t: %d, b: %d",
//                          g_CurWordRect.left,
//                          g_CurWordRect.right,
//                          g_CurWordRect.top,
//                          g_CurWordRect.bottom);
        DbgPrintf("NhWSrh.DLL          return caret place: %d", g_nCurCaretPlaceInTotalWord);
        DbgPrintf("NhWSrh.DLL          g_szTotalWord: %s", g_szTotalWord);
        DbgPrintf("NhWSrh.DLL          Rect l: %d, r: %d, t: %d, b: %d",
                          g_TotalWordRect.left,
                          g_TotalWordRect.right,
                          g_TotalWordRect.top,
                          g_TotalWordRect.bottom);
#endif
#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL ***************** End *****************************");
#endif

        return (DWORD)g_nCurCaretPlaceInTotalWord;    
}

// Hook TextOut and TextOutEx.
DWORD FAR PASCAL _export BL_HookWinApi16()
{
        g_nCurCaretPlace = -1;
        g_szCurWord[0] = 0x00;
        g_szTotalWord[0] = 0x00;
        g_nCurCaretPlaceInTotalWord = -1;
        g_CharType = CHAR_TYPE_OTHER;
        g_bMouseInTotalWord = FALSE;
        g_bAllowGetCurWord = TRUE;
        
        nWordNum = 0;
        szMemDCWordBuff[0] = 0x00;
        
        HookAllTextOut();
        
        return BL_OK;
}

// UnHook TextOut and TextOutEx.
DWORD FAR PASCAL _export BL_UnHookWinApi16()
{
        g_bAllowGetCurWord = FALSE;
        UnHookAllTextOut();
                
        return BL_OK;
}

BOOL bHooked = FALSE;

// Use to hook "TextOut" and "ExtTextOut"
void HookAllTextOut()
{
#ifdef _DEBUG
        DbgPrintf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`hook\n");
#endif
		if (bHooked)
		{
			return;
		}
        bHooked = TRUE;
        HookWinApi(&g_BitBltHook, HOOKANDBACK);
        HookWinApi(&g_TextOutHook, HOOKANDBACK);
        HookWinApi(&g_ExtTextOutHook, HOOKANDBACK);
}

// Use to restore "TextOut" and "ExtTextOut"
void UnHookAllTextOut()
{
#ifdef _DEBUG
	DbgPrintf("|~~~~~~~~~~~~~~~~~~~~~~~unhook~~~~~~~~~\n") ; 
#endif	
        RestoreWinApi(&g_TextOutHook);
        RestoreWinApi(&g_ExtTextOutHook);
        RestoreWinApi(&g_BitBltHook);
        bHooked = FALSE;
}

BOOL WINAPI _export BLTextOut(HDC hDC,int x,int y,LPCSTR lpStr,int cbLen)
{ 
	POINT pt ;
	HWND  hWDC ;
	HWND  hWPT ;
#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL     BLTextOut Begin");
        DbgPrintf("NhWSrh.DLL     BLTextOut hDC: %d", hDC);
        DbgPrintf("NhWSrh.DLL     BLTextOut hDC: %d, x: %d, y: %d, cbLen: %d", 
        		  hDC,
        		  x,
        		  y,
        		  cbLen);
#endif
        // restore the old textout.
        RestoreWinApi(&g_TextOutHook);

	//Added by XGL, Nov 3rd, 1998
	//We cannot get corret word with Explorer as background window
	//in Windows98, so this situation must be dealt with.
	pt.x = g_CurMousePos.x; pt.y = g_CurMousePos.y ;
	hWDC = WindowFromDC(hDC) ;
	hWPT = WindowFromPoint(pt) ;
	
	if (hWDC == NULL || hWPT == hWDC
		|| IsParentOrSelf(hWPT, hWDC)
		|| IsParentOrSelf(hWDC, hWPT))
	{
	//Adding ends. XGL, Nov 3rd, 1998
        if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpStr, cbLen)) && (cbLen > 0))
        {
                g_nTextAlign = GetTextAlign(hDC);
                g_nExtra     = GetTextCharacterExtra(hDC);
                GetCurrentPositionEx(hDC, &g_CurPos);
                GetTextMetrics(hDC, &g_tm);
        
///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// ����޸��ڼ��� TA_CENTER �����ʧ��
	g_dwDCOrg = 0;
		bRecAllRect = FALSE;
		GetStringRect(hDC, (LPSTR)lpStr, cbLen, x, y, &g_rcTotalRect, NULL);
		bRecAllRect = TRUE;
// End Modify
///////////////////////////////////////////////////////////////////////////

                if ((WindowFromDC != NULL)&&(WindowFromDC(hDC) == NULL))
                {
#ifdef _DEBUG
                        DbgPrintf("NhWSrh.DLL     BLTextOut WindowFromDC() = NULL, string add to TextOutBuffer");
                        DbgLPCSTR("NhWSrh.DLL     BLTextOut ", (LPSTR)lpStr, cbLen, TRUE);
#endif
                        // ���㣬��춱���MEMDC�Դ�λ�õ�Ӱ�졤
                        g_dwDCOrg = 0;

                        AddToTextOutBuffer(hDC, (LPSTR)lpStr, cbLen, x, y, NULL);
                }
                else
                {
#ifdef _DEBUG
                        DbgPrintf("NhWSrh.DLL     BLTextOut WindowFromDC() != NULL, string add to TextOutBuffer");
                        DbgLPCSTR("NhWSrh.DLL     BLTextOut ", (LPSTR)lpStr, cbLen, TRUE);
#endif
                        g_dwDCOrg    = GetDCOrg(hDC);
        
                        GetCurMousePosWord(hDC, (LPSTR)lpStr, cbLen, x, y, NULL);
                }
        }
        else
        {
#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL     BLTextOut ((!g_bAllowGetCurWord) || (IsBadReadPtr(lpStr, cbLen)) && (cbLen <= 0))");
        if (!g_bAllowGetCurWord)
        {
        	DbgPrintf("NhWSrh.DLL     BLTextOut !g_bAllowGetCurWord");
        }
        if (IsBadReadPtr(lpStr, cbLen))
        {
    	    DbgPrintf("NhWSrh.DLL     BLTextOut IsBadReadPtr()");
        }
        if (cbLen <= 0)
        {
	        DbgPrintf("NhWSrh.DLL     BLTextOut cbLen <= 0");
        }
#endif
    }
    }
        // call old textout.
        TextOut(hDC, x, y, lpStr, cbLen);
        HookWinApi(&g_TextOutHook, ONLYHOOK);
#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL     BLTextOut End");
#endif
        return (TRUE);
}         

BOOL WINAPI _export BLExtTextOut(HDC hDC,
                                 int x,
                                 int y,
                                 UINT fuOpt,
                                 const RECT FAR* lprc,
                                 LPCSTR lpStr,
                                 UINT cbLen,
                                 int FAR* lpDx)
{
	POINT pt ;
	HWND hWDC ;
	HWND hWPT ;
	
#ifdef _DEBUG
    DbgPrintf("NhWSrh.DLL     BLExtTextOut Begin");
    DbgPrintf("NhWSrh.DLL     BLExtTextOut hDC: %d, x: %d, y: %d, fuOpt: %d, cbLen: %d", 
        	  hDC,
        	  x,
        	  y,
        	  fuOpt,
        	  cbLen);
#endif
        // restore the old textout.
        RestoreWinApi(&g_ExtTextOutHook);
    
	//Added by XGL, Nov 3rd, 1998
	//We cannot get corret word with Explorer as background window
	//in Windows98, so this situation must be dealt with.
	pt.x = g_CurMousePos.x; pt.y = g_CurMousePos.y ;
	hWDC = WindowFromDC(hDC) ;
	hWPT = WindowFromPoint(pt) ;

	if (hWDC == NULL || hWPT == hWDC
		|| IsParentOrSelf(hWPT, hWDC)
		|| IsParentOrSelf(hWDC, hWPT))
	{
	//Adding ends. XGL, Nov 3rd, 1998

		if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpStr, cbLen)) && (cbLen > 0))
        {
                g_nTextAlign = GetTextAlign(hDC);
                g_nExtra     = GetTextCharacterExtra(hDC);
                GetCurrentPositionEx(hDC, &g_CurPos);
                GetTextMetrics(hDC, &g_tm);
///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// ����޸��ڼ��� TA_CENTER �����ʧ��
	g_dwDCOrg = 0;
		bRecAllRect = FALSE;
		GetStringRect(hDC, (LPSTR)lpStr, cbLen, x, y, &g_rcTotalRect, lpDx);
		bRecAllRect = TRUE;
// End Modify
///////////////////////////////////////////////////////////////////////////

                if ((WindowFromDC != NULL)&&(WindowFromDC(hDC) == NULL))
                {
#ifdef _DEBUG
                        DbgPrintf("NhWSrh.DLL     BLTextOut WindowFromDC() == NULL");
                        DbgLPCSTR("NhWSrh.DLL     BLTextOut: ", (LPSTR)lpStr, cbLen, TRUE);
#endif
                        // ���㣬��춱���MEMDC�Դ�λ�õ�Ӱ�졤
                        g_dwDCOrg = 0;
                        
                        AddToTextOutBuffer(hDC, (LPSTR)lpStr, cbLen, x, y, lpDx);
                }
                else
                {
#ifdef _DEBUG
                        DbgPrintf("NhWSrh.DLL     BLTextOut WindowFromDC() != NULL");
                        DbgLPCSTR("NhWSrh.DLL     BLTextOut: ", (LPSTR)lpStr, cbLen, TRUE);
#endif
                        g_dwDCOrg    = GetDCOrg(hDC);
        
                        GetCurMousePosWord(hDC, (LPSTR)lpStr, cbLen, x, y, lpDx);
                }
		}
		else
		{
#ifdef _DEBUG
			DbgPrintf("NhWSrh.DLL     BLExtTextOut ((!g_bAllowGetCurWord) || (IsBadReadPtr(lpStr, cbLen)) && (cbLen <= 0))");
			if (!g_bAllowGetCurWord)
			{
				DbgPrintf("NhWSrh.DLL     BLExtTextOut !g_bAllowGetCurWord");
			}
			if (IsBadReadPtr(lpStr, cbLen))
			{
				DbgPrintf("NhWSrh.DLL     BLExtTextOut IsBadReadPtr()");
			}
			if (cbLen <= 0)
			{
				DbgPrintf("NhWSrh.DLL     BLExtTextOut cbLen <= 0");
			}
#endif
		}
    }
        // call old textout.
        ExtTextOut(hDC, x, y, fuOpt, lprc, lpStr, cbLen, lpDx);
        HookWinApi(&g_ExtTextOutHook, ONLYHOOK);
#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL     BLExtTextOut End");
#endif
        return (TRUE);
}         

BOOL WINAPI _export BLBitBlt(HDC hdcDest,
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

#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL     BLBitBlt Begin");
#endif

        // restore the old textout.
        RestoreWinApi(&g_BitBltHook);
    
#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL     BitBlt: hdcDest: %d, hdcSrc: %d, nXDest: %d, nYDest, %d, nWidth: %d, nHeight: %d",
                          hdcDest,
                          hdcSrc,
                          nXDest,
                          nYDest,
                          nWidth,
                          nHeight);
#endif
        
        if (nWidth > 5)
        {
                g_dwDCOrg = GetDCOrg(hdcDest);
                x = LOWORD(g_dwDCOrg);
                y = HIWORD(g_dwDCOrg);
                x += nXDest;
                y += nYDest;
                g_dwDCOrg = MAKELONG(x, y);
                
                CheckMemDCWordBuffer(hdcDest, hdcSrc);
        }
		else
		{
#ifdef _DICTING_
			if (CheckDCWndClassName(hdcDest))
			{
#ifdef _DEBUG
				DbgPrintf("NhWSrh.DLL          In IE4");
#endif
                g_dwDCOrg = GetDCOrg(hdcDest);
                x = LOWORD(g_dwDCOrg);
                y = HIWORD(g_dwDCOrg);
                x += nXDest;
                y += nYDest;
                g_dwDCOrg = MAKELONG(x, y);
                
                CheckMemDCWordBuffer(hdcDest, hdcSrc);
			}
#endif
		}
        
        // call old textout.
        BitBlt(hdcDest, 
                   nXDest, 
                   nYDest, 
                   nWidth, 
                   nHeight, 
                   hdcSrc, 
                   nXSrc,
                   nYSrc,
                   dwRop);

        HookWinApi(&g_BitBltHook, ONLYHOOK);

#ifdef _DEBUG
        DbgPrintf("NhWSrh.DLL     BLBitBlt End");
#endif
        return (TRUE);
}         

void FAR PASCAL _export Nh16_AddToTotalWord(LPSTR  szBuff,
					                        int    cbLen,
					                        int    nBegin,
					                        int    nEnd,
                    					    int    nCharType,
                         					LPRECT lpStringRect,
                         					BOOL   bInCurWord,
                         					int    nCurCaretPlace)
{
#ifdef _DEBUG
	DbgPrintf("NhW16.dll       ExtTextOutW: %s", szBuff);
	DbgPrintf("NhW16.dll       ExtTextOutW: Rect l: %d, r: %d, t: %d, b: %d",
			  lpStringRect->left,
			  lpStringRect->right,
			  lpStringRect->top,
			  lpStringRect->bottom);
	DbgPrintf("NhW16.dll       ExtTextOutW: Mouse point : x: %d, y: %d",
				g_CurMousePos.x,
				g_CurMousePos.y);
#endif
	g_nCurCaretPlace = nCurCaretPlace;
	AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, *lpStringRect, bInCurWord);
}

int FAR PASCAL _export Nh16_GetTempLen()
{
#ifdef _DEBUG
	DbgPrintf("NhW16.dll       ExtTextOutW: Nh16_GetTempLen: %s", szMemDCWordBuff);
#endif
	return strlen(szMemDCWordBuff);
}

BOOL FAR PASCAL _export Nh16_AddToTempBuff(LPSTR  szBuff,
					                       int    cbLen)
{
	int  nLen;
	
#ifdef _DEBUG
	DbgPrintf("NhW16.dll       ExtTextOutW: Nh16_AddToTempBuff: Before Add: %s", szMemDCWordBuff);
#endif
	if (cbLen >= (int)(BUFFERLENGTH - strlen(szMemDCWordBuff) - 2))
	{
		// buffer too small.
		return FALSE;
	}

	// Write string to buffer.
	nLen = strlen(szMemDCWordBuff);
	strncpy(szMemDCWordBuff + nLen, szBuff, cbLen);
	szMemDCWordBuff[nLen + cbLen    ] = ' ';
	szMemDCWordBuff[nLen + cbLen + 1] = 0x00;
	
#ifdef _DEBUG
	DbgPrintf("NhW16.dll       ExtTextOutW: Nh16_AddToTempBuff: after Add: %s", szMemDCWordBuff);
#endif
	return TRUE;
}					                       

BOOL FAR PASCAL _export Nh16_AddToWordStruct(int nBegin,
											 int nEnd,
											 HDC hMemDC, 
											 int CharType, 
											 LPRECT lpStringRect)
{
#ifdef _DEBUG
	int i;
	DbgPrintf("NhW16.dll       ExtTextOutW: Nh16_AddToWordStruct begin.");
#endif
//////////////////////////////////////////////////////////////
// Modified by Yan/gang 1998.02.20
// Reason: 
//         It cause the system destroy when the user run "WBLASTER"
//         The reason is the the array is overflow.
//         The WordBuffer array size is MEMDC_MAXNUM.
//         But it's point (nWordNum) is large than MEMDC_MAXNUM,
//         also, nWordNum is small than MAXNUM.
//	if (nWordNum >= MAXNUM)
	if (nWordNum >= MEMDC_MAXNUM)
		return FALSE;
// Modify end.
//////////////////////////////////////////////////////////////

	WordBuffer[nWordNum].nBegin = nBegin;
	WordBuffer[nWordNum].nEnd   = nEnd;
	WordBuffer[nWordNum].hMemDC = hMemDC;
	WordBuffer[nWordNum].CharType = CharType;
	WordBuffer[nWordNum].wordRect.left   = lpStringRect->left;
	WordBuffer[nWordNum].wordRect.right  = lpStringRect->right;
	WordBuffer[nWordNum].wordRect.top    = lpStringRect->top;
	WordBuffer[nWordNum].wordRect.bottom = lpStringRect->bottom;
		
	nWordNum++;
	
#ifdef _DEBUG
	for (i = 0; i < nWordNum; i++)
	{
		DbgPrintf("NhW16.dll       nBegin: %d, nEnd: %d, l: %d, r: %d, t: %d, b: %d",
				  WordBuffer[i].nBegin,
				  WordBuffer[i].nEnd,
				  WordBuffer[i].wordRect.left,
				  WordBuffer[i].wordRect.right,
				  WordBuffer[i].wordRect.top,
				  WordBuffer[i].wordRect.bottom);
	}
	DbgPrintf("NhW16.dll       ExtTextOutW: Nh16_AddToWordStruct end.");
#endif
	return TRUE;
}											 

BOOL FAR PASCAL _export Nh16_AddToCharStruct(int nBeignPlace, int nLeft, int nRight)
{
#ifdef _DEBUG
	DbgPrintf("NhW16.dll       ExtTextOutW: Nh16_AddToCharStruct, nBeginPlace: %d, nleft: %d, nRight: %d",
			  nBeignPlace,
			  nLeft,
			  nRight);
#endif
	pnMemDCCharLeft[nBeignPlace]  = nLeft;
	pnMemDCCharRight[nBeignPlace] = nRight;
	
	return TRUE;
}
					                       
//==================================
// PHYS - Matt Pietrek 1995
// FILE: PHYS16.C
//==================================


static char MS_DOS_STR[] = "MS-DOS";

//
// Return a far pointer to the LDT
//
unsigned short GetLDTAlias(void)
{
    unsigned short  LDT_alias;
    unsigned short  (far * dpmiproc)(void);

    //
    // Use INT 2Fh, fn. 168A to get the "DPMI extensions
    // entry point" function pointer
    //
    _asm     mov     si, offset MS_DOS_STR   // DS:SI = "MS-DOS"
    _asm     mov     ax, 168Ah
    _asm     int     2Fh
    _asm     cmp     al, 8Ah
    _asm     je      extensions_not_found

    //
    // The entry point is returned in ES:DI.  Save it
    //
    _asm     mov     word ptr [dpmiproc], di
    _asm     mov     word ptr [dpmiproc+2], es

    //
    // Call the extensions with AX == 0x100.  The LDT alias
    // selector is return in AX.  Carry flag is set on failure.
    //
    _asm     mov     ax, 100h
    LDT_alias = dpmiproc();
    _asm     jc      extensions_not_found;

    return  LDT_alias;

extensions_not_found:   // We get here if something failed
    return  0;
}

WORD FAR PASCAL __loadds GetRing0Callgate( DWORD func_address,
											unsigned cParams )
{
    CALLGATE_DESCRIPTOR far *callgate_desc;
    CODE_SEG_DESCRIPTOR far *ring0_desc;
    unsigned short ldt_alias;
    unsigned short ring_0_alias;
    unsigned short callgate_selector;

    if ( (ldt_alias = GetLDTAlias()) == 0 )
        return 0;

    //
    // Grab a selector from Windows to use as the CS at ring 0
    //
    if ( !(ring_0_alias = AllocSelector(0)) )
        return 0;

    //
	// Set up the fields in the descriptor to be a ring 0, flat model seg
    //
    ring0_desc = MAKELP( ldt_alias, ring_0_alias & 0xFFF8 );
	ring0_desc->limit_0_15 = 0xFFFF;
	ring0_desc->base_0_15 = 0;
	ring0_desc->base_16_23 = 0;
	ring0_desc->readable = 1;
	ring0_desc->conforming = 0;
	ring0_desc->code_data = 1;
	ring0_desc->app_system = 1;
    ring0_desc->dpl = 0;
    ring0_desc->present = 1;
	ring0_desc->limit_16_19 = 0xF;
	ring0_desc->always_0 = 0;
	ring0_desc->seg_16_32 = 1;
	ring0_desc->granularity = 1;
	ring0_desc->base_24_31 = 0;
	
    //
    // Allocate the selector that'll be used for the call gate
    //
    if ( (callgate_selector= AllocSelector(0)) == 0 )
    {
        FreeSelector( ring_0_alias );
        return 0;
    }

    //
    // Create a pointer to the call gate descriptor
    //
    callgate_desc = MAKELP( ldt_alias, callgate_selector & 0xFFF8 );

    //
    // Fill in the fields of the call gate descriptor with the
    // appropriate values for a 16 bit callgate.
    //
    callgate_desc->offset_0_15 = LOWORD( func_address );
    callgate_desc->selector = ring_0_alias;
    callgate_desc->param_count = cParams;
    callgate_desc->some_bits = 0;
    callgate_desc->type = 0xC;          // 386 call gate
    callgate_desc->app_system = 0;      // A system descriptor
    callgate_desc->dpl = 3;             // Ring 3 code can call
    callgate_desc->present = 1;
    callgate_desc->offset_16_31 = HIWORD(func_address);

    return callgate_selector;
}

BOOL FAR PASCAL __loadds FreeRing0Callgate( WORD callgate )
{
    CALLGATE_DESCRIPTOR far *callgate_desc;
    unsigned short ldt_alias;

    if ( (ldt_alias = GetLDTAlias()) == 0 )
        return FALSE;

    //
    // Create a pointer to the call gate descriptor
    //
    callgate_desc = MAKELP( ldt_alias, callgate & 0xFFF8 );

    //
    // First, free the ring 0 alias selector stored in the LDT
    // call gate descriptor, then free the call gate selector.
    //
    FreeSelector( callgate_desc->selector );
    FreeSelector( callgate );
}

//Added by XGL, Dec 8th, 1998
BOOL IsParentOrSelf(HWND hParent, HWND hChild)
{
	HWND hTemp = hChild;
	HWND hDesktop ;
	
	if (hParent == NULL || hChild == NULL)
	{
		return FALSE ;
	}

	hDesktop = GetDesktopWindow() ;
	while (hTemp != NULL && hTemp != hDesktop)
	{
		if (hTemp == hParent)
		{
			return TRUE ;
		}

		hTemp = GetParent(hTemp) ;
	}

	return FALSE ;
}
//Adding ends.
