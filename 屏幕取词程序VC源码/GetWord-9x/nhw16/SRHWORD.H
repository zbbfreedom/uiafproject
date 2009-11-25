/* srhword.h*/
#ifndef __SRHWORD_H__
#define __SRHWORD_H__

#include<windows.h>
#include "..\pub3216.h"

#define _DICTING_

typedef HWND (WINAPI *WINDOWFROMDC_PROC)(HDC);

typedef struct _tagWordPara
{
	HDC  hMemDC;
	int  nBegin;
	int  nEnd;
	int  CharType;
	RECT wordRect;
}WORDPARA, *LPWORDPARA;

// USE FOR GET A WORD IN SCREEN.
#define WORK_GETWORD    0
// USE FOR GET ALL WORD IN WINDOW.
#define WORK_NOGETWORD  1

#define BL_NO_WORD -1

int CALLBACK LibMain(HINSTANCE hInst, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
int CALLBACK WEP(int nExitType);

DWORD FAR PASCAL _export BL_SetVer16(int nLangVer);
DWORD FAR PASCAL _export BL_SetGetWordStyle(int nGetWordStyle);
DWORD FAR PASCAL _export BL_SetPara16(short hNotifyWnd, short MouseX, short MouseY);
DWORD FAR PASCAL _export BL_GetBuffer16(LPSTR lpszBuffer, short nBufferSize, LPWORDRECT lpWr);
DWORD FAR PASCAL _export BL_HookWinApi16();
DWORD FAR PASCAL _export BL_UnHookWinApi16();

BOOL WINAPI _export BLTextOut(HDC hDC,int x,int y,LPCSTR lpStr,int cbLen);
BOOL WINAPI _export BLExtTextOut(HDC hDC, int x, int y, UINT fuOpt, const RECT FAR* lprc, LPCSTR lpStr,
                                 UINT cbLen,
                                 int FAR* lpDx);
BOOL WINAPI _export BLBitBlt(HDC hdcDest,
                             int nXDest,
                             int nYDest,
                             int nWidth,
                             int nHeight,
                             HDC hdcSrc,
                             int nXSrc,
                             int nYSrc,
                             DWORD dwRop);

/*
DWORD FAR PASCAL _export BL_ExtTextOutW(HDC hDC,
										int X,
										int Y,
										UINT fuOptions,
										LPRECT lprc,
										LPSTR lpString,
										UINT cbCount,
										LPINT lpDx);
*/
void FAR PASCAL _export Nh16_AddToTotalWord(LPSTR  szBuff,
					                        int    cbLen,
					                        int    nBegin,
					                        int    nEnd,
                    					    int    nCharType,
                         					LPRECT lpStringRect,
                         					BOOL   bInCurWord,
                         					int    nCurCaretPlace);


int FAR PASCAL _export Nh16_GetTempLen();
BOOL FAR PASCAL _export Nh16_AddToTempBuff(LPSTR  szBuff,
					                       int    cbLen);
BOOL FAR PASCAL _export Nh16_AddToWordStruct(int nBegin,
											 int nEnd,
											 HDC hMemDC, 
											 int CharType, 
											 LPRECT lpStringRect);
BOOL FAR PASCAL _export Nh16_AddToCharStruct(int nBeignPlace, int nLeft, int nRight);

void HookAllTextOut();
void UnHookAllTextOut();

//==================================
// PHYS - Matt Pietrek 1995
// FILE: DESCRIPT.H
//==================================

//#pragma pack(1)

typedef struct
{
    unsigned short  limit_0_15;
    unsigned short  base_0_15;
    unsigned char   base_16_23;

    unsigned int    accessed    : 1;
    unsigned int    readable    : 1;
    unsigned int    conforming  : 1;
    unsigned int    code_data   : 1;
    unsigned int    app_system  : 1;
    unsigned int    dpl         : 2;
    unsigned int    present     : 1;

    unsigned int    limit_16_19 : 4;
    unsigned int    unused      : 1;
    unsigned int    always_0    : 1;
    unsigned int    seg_16_32   : 1;
    unsigned int    granularity : 1;

    unsigned char   base_24_31;
} CODE_SEG_DESCRIPTOR;

typedef struct
{
    unsigned short  offset_0_15;
    unsigned short  selector;

    unsigned int    param_count : 4;
    unsigned int    some_bits   : 4;

    unsigned int    type        : 4;
    unsigned int    app_system  : 1;
    unsigned int    dpl         : 2;
    unsigned int    present     : 1;

    unsigned short  offset_16_31;
} CALLGATE_DESCRIPTOR;

unsigned short GetLDTAlias(void);

typedef void (_far _pascal * GENERIC_PROC)(void);

GENERIC_PROC CreateCallgate(void far * func_address, unsigned params);
void FreeCallgate( GENERIC_PROC callgate_ptr );

WORD FAR PASCAL __loadds GetRing0Callgate( DWORD func_address,
											unsigned cParams );
BOOL FAR PASCAL __loadds FreeRing0Callgate( WORD callgate );

//Added by XGL, Dec 8th, 1998
BOOL IsParentOrSelf(HWND hParent, HWND hChild) ;
//Adding ends.

#endif //__SRHWORD_H__     
