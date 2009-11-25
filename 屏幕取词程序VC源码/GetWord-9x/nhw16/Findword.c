#include <windows.h>
#include "srhword.h"
#include "findword.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "..\pub3216.h"
#include <malloc.h>

#ifdef _DEBUG
#include "DbgFunc.h"
#endif //_DEBUG

extern int   g_nCurrWindowVer;

extern char  g_szTotalWord[BUFFERLENGTH];
extern RECT  g_TotalWordRect;
extern int   g_CharType;
extern int   g_nCurCaretPlaceInTotalWord;
extern int   g_bMouseInTotalWord;
//Added by XGL,Sep 29th,1998
extern RECT  g_rcFirstWordRect;	
//Adding ends.


extern char  g_szCurWord[BUFFERLENGTH];
extern RECT  g_CurWordRect;
extern int   g_nCurCaretPlace;

extern char szMemDCWordBuff[BUFFERLENGTH];
extern int  pnMemDCCharLeft[BUFFERLENGTH];
extern int  pnMemDCCharRight[BUFFERLENGTH];
extern WORDPARA WordBuffer[MEMDC_MAXNUM];
extern int nWordNum;

extern BOOL  g_bAllowGetCurWord;
extern POINT g_CurMousePos;
extern HWND  g_hNotifyWnd;

extern UINT         g_nTextAlign;
extern DWORD        g_dwDCOrg;
extern int          g_nExtra;
extern POINT        g_CurPos;
extern TEXTMETRIC   g_tm;

///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// 用於修改在计算 TA_CENTER 情况的失误。
extern BOOL bRecAllRect;
extern RECT g_rcTotalRect;
// End Modify
///////////////////////////////////////////////////////////////////////////


#define RIGHT  1
#define LEFT  -1

#ifdef _DICTING_
///////////////////////////////////////////////////////////////////////////
// Add by Yan/Gang   11/13/1997
// 用於向查字助理通知取得所需要的单词·
extern UINT BL_HASSTRING;
// Add end.
///////////////////////////////////////////////////////////////////////////
#endif

#ifdef _DICTING_
/////////////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/19
// 主要解决通配符的问题。
extern int g_nGetWordStyle;
// End Add.
/////////////////////////////////////////////////////////////////////////////////
//Added by XGL Sep 17th, 1998
extern int g_nWordsInPhrase ;
extern BOOL g_bPhraseNeeded ;
extern int g_nPhraseCharType ;
//Adding ends
/////////////////////////////////////////////////////////////////////////////////
#endif

//////////////////////////////////////////////////////////////////////////////////
// TODO: Return a char type.
__inline int  GetCharType(char ch)
{
	BYTE chitem = ch;

	if (ch < 0)
		return CHAR_TYPE_HZ;

	if (((ch >= 'a')&&(ch <= 'z'))||
	    ((ch >= 'A')&&(ch <= 'Z')))
	{
		return CHAR_TYPE_ASCII;
	}
	
	return CHAR_TYPE_OTHER;	
}

#ifdef _DICTING_
/////////////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/19
// 主要解决通配符的问题。
__inline int  FindAWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	return FindTWWord(lpString, nFromPlace, nLength);
}


__inline int  FindDWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_ASCII)
		{
			i++;
		}
		else
		{
			return i-1;
		}
	} 
	
	return nLength - 1;
}

__inline int  FindTWWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
//	int j;
	while (i < nLength)
	{
		if (lpString[i] == CHAR_LINK)
		{
			if (IsASCIIWord(lpString, nFromPlace, nLength, i + 1))
			{
				i++;
			}
			else
			{
				return i-1;
			}
		}
		else
		{
			// 用於处理 '-' 的几种情况·
			if (IsASCIIWord(lpString, nFromPlace, nLength, i))
			{
				i++;
			}
			else
			{
				return i-1;
			}
		}
	} 
	
	return nLength - 1;
}

__inline BOOL IsASCIIWord(LPCSTR lpString, int nFromPlace, int nLength, int nCurCharNum)
{
	if (GetCharType(lpString[nCurCharNum]) == CHAR_TYPE_ASCII)
	{
		return TRUE;
	}
	return FALSE;
}
// End Add.
/////////////////////////////////////////////////////////////////////////////////
#else
__inline int  FindAWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (lpString[i] == CHAR_LINK)
		{
			if (IsASCIIWord(lpString, nFromPlace, nLength, i + 1))
			{
				i++;
			}
			else
			{
				return i-1;
			}
		}
		else
		{
			// 用於处理 '-' 的几种情况·
			if (IsASCIIWord(lpString, nFromPlace, nLength, i))
			{
				i++;
			}
			else
			{
				return i-1;
			}
		}
	} 
	
	return nLength - 1;
}

#define CHAR_WILDCHAR1 '*'
#define CHAR_WILDCHAR2 '?'

__inline BOOL IsASCIIWord(LPCSTR lpString, int nFromPlace, int nLength, int nCurCharNum)
{
	if (GetCharType(lpString[nCurCharNum]) == CHAR_TYPE_ASCII)
	{
		return TRUE;
	}
    
	return FALSE;
}
#endif

__inline int  FindHZWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;

	if ((BYTE)(lpString[nFromPlace]) >= 0xa1
		&& (BYTE)(lpString[nFromPlace]) <= 0xa9)
	{
		return nFromPlace + 1 ;
	}

	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_HZ)
		{
			 //Added by XGL, Nov 13th, 1998
			 //We have to filter out Chinese punctuation marks,
			 //which is diffent as it is in ASCII.
			 if ((BYTE)(lpString[i]) >= 0xa1
				 && (BYTE)(lpString[i]) <= 0xa9)
			 {
				 return i - 1 ;
			 }
			 //Adding ends. XGL, Nov 13th, 1998

			i = i + 2;
		}
		else
		{
			return i - 1;
		}
	} 
	
	return nLength - 1;
}

__inline int FindNextWordBegin(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_OTHER)
		{
			i++;
		}
		else
		{
			return i-1;
		}
	} 
	
	return i - 1;
}

__inline int GetCurWordEnd(LPCSTR lpString, int nFromPlace, int nLength, int nCharType)
{
	switch (nCharType)
	{                       
		case CHAR_TYPE_ASCII:
			 return FindAWord(lpString, nFromPlace, nLength);
			 break;
		case CHAR_TYPE_HZ:
			 return FindHZWord(lpString, nFromPlace, nLength);
			 break;
		case CHAR_TYPE_OTHER:
			 return FindNextWordBegin(lpString, nFromPlace, nLength);
			 break;
	}
	return FindAWord(lpString, nFromPlace, nLength);
}

__inline void CopyWord(LPSTR lpWord, LPCSTR lpString, int nBegin, int nEnd)
{
	int i;
	for ( i = nBegin; i <= nEnd; i++)
	{
		lpWord[i - nBegin] = lpString[i];
	}
	lpWord[nEnd - nBegin + 1] = '\0';
}

__inline void GetStringTopBottom(HDC hDC, int y, RECT* lpStringRect)
{
	POINT  WndPos;

	WndPos.y = HIWORD(g_dwDCOrg);

    if (TA_UPDATECP & g_nTextAlign)
    {
    	y = g_CurPos.y;
    }
    
	switch ((TA_TOP | TA_BOTTOM)&g_nTextAlign)
	{
		case TA_BOTTOM:
			 lpStringRect->top    = y - g_tm.tmHeight + g_tm.tmInternalLeading;
			 lpStringRect->bottom = y;
			 break;
		case TA_BASELINE:
			 lpStringRect->top    = y - g_tm.tmAscent;
			 lpStringRect->bottom = y + g_tm.tmDescent;
			 break;
		case TA_TOP:
		default:
			 lpStringRect->top    = y;
			 lpStringRect->bottom = y + g_tm.tmHeight + g_tm.tmInternalLeading;
			 break;
	}
	
	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);

	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
}

__inline void GetStringLeftRight(HDC hDC, LPSTR szBuff, int cbLen, int x, RECT* lpStringRect, int FAR* lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    int i;

	if (cbLen < 0)
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}

	GetTextExtentPoint(hDC, szBuff, cbLen, &StringSize);
	WndPos.x = LOWORD(g_dwDCOrg);
	
	if (lpDx != NULL)
	{
		StringSize.cx = 0;
		for (i = 0; i < cbLen; i++)
		{
			StringSize.cx += lpDx[i];
		}
	}
    
    if (TA_UPDATECP & g_nTextAlign)
    {
    	x = g_CurPos.x;
    }
    
	switch ((TA_LEFT | TA_CENTER | TA_RIGHT)&g_nTextAlign)
	{
		case TA_RIGHT:
			 if (!bRecAllRect)
			 {
				 lpStringRect->right = x;
				 lpStringRect->left  = x - StringSize.cx;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_CENTER:
			 if (!bRecAllRect)
			 {
				 lpStringRect->right = x + StringSize.cx / 2;
				 lpStringRect->left  = x - StringSize.cx / 2;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_LEFT:
		default:
			 lpStringRect->left  = x ;
			 lpStringRect->right = x + StringSize.cx;
			 break;
	}
	
	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);

	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

//#define _TEST

void GetStringRect(HDC hDC, LPSTR szBuff, int cbLen, int x, int y, RECT* lpStringRect, int FAR* lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    int i;

	if (cbLen < 0)
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}

	GetTextExtentPoint(hDC, szBuff, cbLen, &StringSize);
	WndPos.x = LOWORD(g_dwDCOrg);
	WndPos.y = HIWORD(g_dwDCOrg);
	
	if (lpDx != NULL)
	{
		StringSize.cx = 0;
		for (i = 0; i < cbLen; i++)
		{
			StringSize.cx += lpDx[i];
		}
	}
    
    if (TA_UPDATECP & g_nTextAlign)
    {
    	x = g_CurPos.x;
    	y = g_CurPos.y;
    }
    
	switch ((TA_LEFT | TA_CENTER | TA_RIGHT)&g_nTextAlign)
	{
		case TA_RIGHT:
			 if (bRecAllRect == FALSE)
			 {
				 lpStringRect->right = x;
				 lpStringRect->left  = x - StringSize.cx;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_CENTER:
			 if (bRecAllRect == FALSE)
			 {
				 lpStringRect->right = x + StringSize.cx / 2;
				 lpStringRect->left  = x - StringSize.cx / 2;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_LEFT:
		default:
			 lpStringRect->left  = x ;
			 lpStringRect->right = x + StringSize.cx;
			 break;
	}
	
	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE)&g_nTextAlign)
	{
		case TA_BOTTOM:
			 lpStringRect->top    = y - StringSize.cy;
			 lpStringRect->bottom = y;
			 break;
		case TA_BASELINE:
			 lpStringRect->top    = y - g_tm.tmAscent;
			 lpStringRect->bottom = y + g_tm.tmDescent;
			 break;
		case TA_TOP:
		default:
			 lpStringRect->top    = y;
			 lpStringRect->bottom = y + StringSize.cy;
			 break;
	}

	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);

	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

DWORD GetCurMousePosWord(HDC   hDC, 
						 LPSTR szBuff, 
						 int   cbLen, 
						 int   x, 
						 int   y, 
						 int   FAR* lpDx)
{
	int   nCurrentWord, nPrevWord;
	RECT  StringRect;
	int   CharType;
	int   nLeft;
	DWORD dwReturn;

	GetStringTopBottom(hDC, y, &StringRect);
	if ((StringRect.top > g_CurMousePos.y) || (StringRect.bottom < g_CurMousePos.y))
	{
		return NO_CURMOUSEWORD;
	}

	GetStringRect(hDC, szBuff, cbLen, x, y, &StringRect, lpDx);
	nLeft = StringRect.left;
//	nLeft = 0;

	nPrevWord = nCurrentWord = -1;
	while (nCurrentWord < cbLen)
	{
		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);
		dwReturn     = CheckMouseInCurWord(hDC, szBuff, cbLen, x, y, lpDx, &nLeft, nPrevWord + 1, nCurrentWord, CharType);

#ifdef _DICTING_
		if (dwReturn == HAS_CURMOUSEWORD)
		{
			if (CharType == CHAR_TYPE_OTHER)
			{
				PostMessage(g_hNotifyWnd, BL_HASSTRING, 0, 0l);
			}
			else
			{
			}
		}
		else
		{
			if (g_bMouseInTotalWord)
			{
				PostMessage(g_hNotifyWnd, BL_HASSTRING, 0, 0l);
			}
		}
#else
		if (dwReturn == HAS_CURMOUSEWORD)
			return HAS_CURMOUSEWORD;
#endif
		
		if (nCurrentWord >= cbLen - 1)
			return NO_CURMOUSEWORD;
	}
		
	return NO_CURMOUSEWORD;   
}

__inline DWORD CheckMouseInCurWord(HDC   hDC, 
								   LPSTR szBuff, 
								   int   cbLen, 
								   int   x, 
								   int   y, 
								   int   FAR* lpDx,
								   int*  lpLeft,
								   int   nBegin,    // = nPrevWord + 1
								   int   nEnd,
								   int   nCharType)
{
	RECT  StringRect;

	GetStringRect(hDC, szBuff, nEnd + 1, x, y, &StringRect, lpDx);
	StringRect.left = *lpLeft;
	*lpLeft = StringRect.right;

	if (  ((g_CurMousePos.x >= StringRect.left    ) && (g_CurMousePos.x <= StringRect.right))
	    || (g_CurMousePos.x == StringRect.left - 1)
	    || (g_CurMousePos.x == StringRect.right + 1))
	{
		switch (nCharType)
		{
			case CHAR_TYPE_HZ:

			case CHAR_TYPE_ASCII:
				 CopyWord(g_szCurWord, szBuff, nBegin, nEnd);
				 g_CurWordRect.left   = StringRect.left;
				 g_CurWordRect.right  = StringRect.right;
				 g_CurWordRect.top    = StringRect.top;
				 g_CurWordRect.bottom = StringRect.bottom;
				 
				 g_nCurCaretPlace = -1;
				 CalculateCaretPlace(hDC, 
									 szBuff, 
									 cbLen,
									 x,
									 y,
									 lpDx,
									 nBegin,
									 nEnd,
									 nCharType);
				 
				 break;
			case CHAR_TYPE_OTHER:
				 break;
		}
		AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, TRUE);
		if (  (nCharType == CHAR_TYPE_OTHER)
		    &&(CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right)))
		{
#ifdef _DICTING_
			return HAS_CURMOUSEWORD;
#else
			return NO_CURMOUSEWORD;
#endif
		}

		return HAS_CURMOUSEWORD;
	}
	else
	{
	}					

	AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, FALSE);

	return NO_CURMOUSEWORD;   
}

// 用於计算指定位置在字串中的位置·
__inline DWORD CalculateCaretPlace(HDC   hDC, 
								   LPSTR szBuff, 
								   int   cbLen, 
								   int   x, 
								   int   y, 
								   int   FAR* lpDx,
								   int   nBegin,    // = nPrevWord + 1
								   int   nEnd,
								   int   nCharType)
{
	RECT  StringRect, BeginRect;
	RECT  CaretPrevRect, CaretNextRect;
//	int   itemWidth; 
	double itemWidth; 
	int   TempPlace;
	int   i;

	if ((nCharType == CHAR_TYPE_HZ) && (nBegin == nEnd))
	{
		g_nCurCaretPlace = -1;
		return 0L;
	}

	GetStringRect(hDC, szBuff, nBegin, x, y, &BeginRect, lpDx);
	
	GetStringRect(hDC, szBuff, nEnd + 1,   x, y, &StringRect, lpDx);
	StringRect.left = BeginRect.right;
    if (StringRect.left == StringRect.right)
    {
		g_nCurCaretPlace = -1;
		return 0L;
    }
	
	switch (nCharType)
	{
		case CHAR_TYPE_HZ:
///////////////////////////////////////////////////////////////////////
// 汉字处理部分·
			 // 由於汉字都是等宽的字符，因此对於汉字的计算是通过计算
			 // 平均宽度来实现的·
//			 itemWidth = (StringRect.right - StringRect.left) / (nEnd - nBegin + 1);
/////////////////////////////////////////////////////////////////
// Modify by GY
// Ver0.036  074号 bug.
// 主要是由於在进行类型转换中有误·			 
			 itemWidth = ((double)StringRect.right - (double)StringRect.left + 1) / ((double)nEnd - (double)nBegin + 1);
// Modify end
////////////////////////////////////////////////////////////////////
//			 g_nCurCaretPlace = (g_CurMousePos.x - StringRect.left) * (nEnd - nBegin + 1) / (StringRect.right - StringRect.left);
			 for (i = 0; i <= (nEnd - nBegin + 1); i++)
			 {
			 	if (CalcCaretInThisPlace(g_CurMousePos.x, (double)((double)StringRect.left + (double)(itemWidth * i))))
			 	{
				 	g_nCurCaretPlace = i;
				 	i = nEnd - nBegin + 2;
			 	}
			 }
             break;
//
///////////////////////////////////////////////////////////////////////             
		case CHAR_TYPE_ASCII:
			 // 由於英文字符为不等宽字符· 按以下步骤来计算的：
			 // 1. 按等宽计算一个相对位置·
			 // 2. 计算该相对位置的的绝对位置·
			 // 3. 若在该位置，返回·
			 // 4. 若该绝对长小於相对位置乘以宽，转6
			 // 5. 向右计算·
			 // 6. 向左计算·
			 
			 // 1. 按等宽计算一个相对位置·
			 // 计算宽度·
			 itemWidth = (StringRect.right - StringRect.left + 1) / (nEnd - nBegin + 1);
			 // 计算位置·
			 TempPlace = (g_CurMousePos.x - StringRect.left) * (nEnd - nBegin + 1) / (StringRect.right - StringRect.left);
			 
			 // 2. 计算该相对位置的的绝对位置·
			 GetStringRect(hDC, szBuff, TempPlace,     x, y, &CaretPrevRect, lpDx);
			 GetStringRect(hDC, szBuff, TempPlace + 1, x, y, &CaretNextRect, lpDx);
			 
			 // 3. 若在该位置，返回·
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = TempPlace - nBegin;
			 	break;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = TempPlace - nBegin + 1;
			 	break;
			 }

			 // 4. 若该绝对长小於相对位置乘以宽，转6
			 if (g_CurMousePos.x > CaretNextRect.right)
			 {
				 // 5. 向右计算·      
				 GetEngLishCaretPlace(hDC, 
									  szBuff,
									  x,
									  y,
									  lpDx,
									  nBegin,    // = nPrevWord + 1
									  nEnd,
									  TempPlace,
									  RIGHT);
			 }
			 else
			 {
				 // 6. 向左计算·
				 GetEngLishCaretPlace(hDC, 
									  szBuff,
									  x,
									  y,
									  lpDx,
									  nBegin,    // = nPrevWord + 1
									  nEnd,
									  TempPlace,
									  LEFT);
			 }
			 
			 break;
	}
	return 0L;
}

__inline DWORD GetEngLishCaretPlace(HDC   hDC, 
									LPSTR szBuff,
									int   x,
									int   y,
									int   FAR* lpDx,
									int   nBegin,    // = nPrevWord + 1
									int   nEnd,
									int   TempPlace,
									int   turnto)
{
	int i;
	RECT  CaretPrevRect, CaretNextRect;
	
	if (turnto == RIGHT)
	{
		i = TempPlace;
		GetStringRect(hDC, szBuff, i, x, y, &CaretPrevRect, lpDx);
		
		for (i = TempPlace; i <= nEnd; i++)
		{
			 GetStringRect(hDC, szBuff, i + 1, x, y, &CaretNextRect, lpDx);
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = i - nBegin;
			 	return 0;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = i - nBegin + 1;
			 	return 0;
			 }

			 CopyRect(&CaretPrevRect, &CaretNextRect);
		}
		g_nCurCaretPlace = nEnd - nBegin + 1;
	}
	else
	{
		i = TempPlace;
		GetStringRect(hDC, szBuff, i + 1, x, y, &CaretNextRect, lpDx);

		for (i = TempPlace; i >= nBegin; i--)
		{
			 GetStringRect(hDC, szBuff, i, x, y, &CaretPrevRect, lpDx);
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = i - nBegin;
			 	return 0;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = i - nBegin + 1;
			 	return 0;
			 }

			 CopyRect(&CaretNextRect, &CaretPrevRect);
		}
		g_nCurCaretPlace = nBegin - nBegin;
	}
	
	return 0;
}

//BOOL CalcCaretInThisPlace(int CaretX, int nPlace)
BOOL CalcCaretInThisPlace(int CaretX, double nPlace)
{
	if (((double)CaretX >= nPlace - 3)&&((double)CaretX <= nPlace + 1))
	{
		return TRUE;
	}

	return FALSE;
}

// Get current Chinese char that under the mouse cursor.
// 1.                       2
// mouse place :    |            |
// chinese char:  英业达      英业达
// return place:    |           |
__inline int GetHZBeginPlace(LPSTR lpszHzBuff, int nBegin, int nEnd, LPRECT lpStringRect)
{
	int itemWidth; 
	int nReturn;

	itemWidth = (lpStringRect->right - lpStringRect->left + 1) / (nEnd - nBegin + 1);
//	nReturn = (g_CurMousePos.x - lpStringRect->left) / itemWidth;
	nReturn = (g_CurMousePos.x - lpStringRect->left) * (nEnd - nBegin + 1) / (lpStringRect->right - lpStringRect->left);

	if (nReturn == 0)
		return nBegin;		
    
	if (nReturn % 2 == 1)
	{
    	lpStringRect->left = lpStringRect->left + (nReturn - 1) * itemWidth;
    	return nBegin + nReturn - 1;
    }
    
   	lpStringRect->left = lpStringRect->left + nReturn * itemWidth;
	return (nBegin + nReturn);
}

//=========================================================
//
//  FUNCTION: void AddToTotalWord(LPSTR, int, int, int, int, RECT, BOOL);
//
//  PURPOSE:  用於将多次文本输出的单词合并为一个完整单词·
//
//  PARAMETER: 
//       
//       LPSTR : 由 TextOut 或 ExtTextOut 函数传递的字符串·
//       int   : 字符串长·
//       int   : 由切词程序处理後的单词在串中的起始位置·( 0 代表串的第一字节 )
//       int   : 由切词程序处理後的单词在串中的结束位置·( 0 代表串的第一字节 )
//       int   : 代表由 nBegin, nEnd 指定的单词的性质 ( 中文／英文／其他字符)
//       RECT  : 代表由 nBegin, nEnd 指定的单词的区域·
//       BOOL  : 用於判断当前鼠标位置是否在该单词中·
//
//  COMMENTS:
//
//
//=========================================================
_inline void AddToTotalWord(LPSTR szBuff, 
							int   cbLen, 
							int   nBegin,
							int   nEnd,
							int   nCharType,
							RECT  StringRect,   
							BOOL  bInCurWord)
{
	int nPos = 0 ; //temp variable

	if ((nCharType == CHAR_TYPE_OTHER) && (!g_bMouseInTotalWord))
	{
		// 处理串开始为空格的情况·
		g_szTotalWord[0] = 0x00;
		g_CharType = nCharType;
		g_bMouseInTotalWord = FALSE;
		return;
	}
	
	//Added by XGL, see GetCurWordEnd for reasons
	if (((BYTE)(szBuff[nBegin]) >= 0xa1 && (BYTE)(szBuff[nBegin]) <= 0xa9)
		&& (!g_bMouseInTotalWord))
	{
		// 处理串开始为空格的情况·
		g_szTotalWord[0] = 0x00;
		g_CharType = CHAR_TYPE_HZ;
		g_bMouseInTotalWord = FALSE;
		return;
	}

	if ((g_szTotalWord[0] == 0x00)&&(nCharType != CHAR_TYPE_OTHER))
	{
		// 如果完整词缓冲区为空，并且单词为中文或英文，
		// 拷贝字符串·
		CopyWord(g_szTotalWord, szBuff, nBegin, nEnd);
		g_TotalWordRect.left   = StringRect.left;
		g_TotalWordRect.right  = StringRect.right;
		g_TotalWordRect.top    = StringRect.top;
		g_TotalWordRect.bottom = StringRect.bottom;
		g_CharType = nCharType;
		if (bInCurWord)
		{
			g_bMouseInTotalWord = TRUE;
			//Added by XGL, Sep 17th, 1998
			//When the word pointed to by mouse was got,
			//attain it's char-type.
			g_nPhraseCharType = nCharType ;
			g_nWordsInPhrase++ ;
			//Adding ends.
			//Added by XGL, Sep 29th, 1998
			g_rcFirstWordRect.left   = StringRect.left;
			g_rcFirstWordRect.right  = StringRect.right;
			g_rcFirstWordRect.top    = StringRect.top;
			g_rcFirstWordRect.bottom = StringRect.bottom;
			//Adding ends.
		}
		else
		{
			g_bMouseInTotalWord = FALSE;
		}
		g_nCurCaretPlaceInTotalWord = -1;
		if (g_nCurCaretPlace != -1)
		{
			g_nCurCaretPlaceInTotalWord = g_nCurCaretPlace;
		}
		return;
	}

	if (!g_bMouseInTotalWord)
	{
		// Added by XGL
		//(nCharType == CHAR_TYPE_OTHER) && (!g_bMouseInTotalWord)
		//See the line above. How can conditions
		//in the following line happen.
		//Adding ends.
/*		if ((nCharType == CHAR_TYPE_OTHER)&&(g_CharType != nCharType))
		{
			// Clear buffer.
			g_szTotalWord[0] = 0x00;
			g_CharType = CHAR_TYPE_OTHER;
			g_bMouseInTotalWord = FALSE;
			g_nCurCaretPlaceInTotalWord = -1;
			if (g_nCurCaretPlace != -1)
			{
				g_nCurCaretPlaceInTotalWord = g_nCurCaretPlace;
			}
			return;
		}
*/
		if (g_CharType != nCharType)
		{
			CopyWord(g_szTotalWord, szBuff, nBegin, nEnd);
			g_TotalWordRect.left   = StringRect.left;
			g_TotalWordRect.right  = StringRect.right;
			g_TotalWordRect.top    = StringRect.top;
			g_TotalWordRect.bottom = StringRect.bottom;
			g_CharType = nCharType;
			if (bInCurWord)
			{
				g_bMouseInTotalWord = TRUE;
				//Added by XGL, Sep 17th, 1998
				//When the word pointed to by mouse was got,
				//attain it's char-type.
				g_nPhraseCharType = nCharType ;
				g_nWordsInPhrase++ ;
				//Adding ends.
				//Added by XGL, Sep 29th, 1998
				g_rcFirstWordRect.left   = StringRect.left;
				g_rcFirstWordRect.right  = StringRect.right;
				g_rcFirstWordRect.top    = StringRect.top;
				g_rcFirstWordRect.bottom = StringRect.bottom;
				//Adding ends.
			}
			else
			{
				g_bMouseInTotalWord = FALSE;
			}
			g_nCurCaretPlaceInTotalWord = -1;
			if (g_nCurCaretPlace != -1)
			{
				g_nCurCaretPlaceInTotalWord = g_nCurCaretPlace;
			}
			return;
		}
	}

	//Added by XGL
	//g_bMouseInTotalWord == FALSE && g_CharType != nCharType
	//has been dealed with.
	//Adding ends.
	if ((g_CharType != nCharType)) 
	{
/*		if (bInCurWord)
		{
			//Added by XGL
			//How can this condition happen
			//Adding ends.
			if (!g_bMouseInTotalWord)
			{
				CopyWord(g_szTotalWord, szBuff, nBegin, nEnd);
				g_TotalWordRect.left   = StringRect.left;
				g_TotalWordRect.right  = StringRect.right;
				g_TotalWordRect.top    = StringRect.top;
				g_TotalWordRect.bottom = StringRect.bottom;
				g_CharType = nCharType;
				g_bMouseInTotalWord = TRUE;
				g_nCurCaretPlaceInTotalWord = g_nCurCaretPlace;
			}
		}
*/
		//Added by XGL
		//now, g_bMouseInTotalWord == TRUE
		//let space char go through since we want to get phrase
		if ( ((nCharType == CHAR_TYPE_OTHER) && (szBuff[nBegin] == ' '))
			 || ((nCharType != CHAR_TYPE_OTHER)
				 && (g_nPhraseCharType == nCharType)) )
		{
			//Modified by XGL, don't allow space char in Chinese
//			if (   g_bPhraseNeeded 
//				   &&(g_nWordsInPhrase > MIN_WORDS_IN_PHRASE)
//				   &&(g_nWordsInPhrase < MAX_WORDS_IN_PHRASE) )
			if (   g_bPhraseNeeded 
				   &&(g_nWordsInPhrase > MIN_WORDS_IN_PHRASE)
				   &&(g_nWordsInPhrase < MAX_WORDS_IN_PHRASE)
				   &&(!(g_szTotalWord[0] < 0 && szBuff[nBegin] == ' ')) )
			{
			}
			else
			{
				g_nWordsInPhrase = MAX_WORDS_IN_PHRASE + 1 ;
				return;
			}
		}
		else
		{
			g_nWordsInPhrase = MAX_WORDS_IN_PHRASE + 1 ;
			return;
		}
		//Adding ends.
	}
	
/*	//Added by XGL, Sep 17th, 1998
	if ( g_bMouseInTotalWord && g_bPhraseNeeded
		 && (strlen(g_szTotalWord) > MAX_WORDS_IN_PHRASE * 2)
		 && (g_nPhraseCharType == CHAR_TYPE_HZ) )
	{
		//We cann't count Chinese words using space char
		//so g_nWordsInPhrase is useless now
		g_nWordsInPhrase = MAX_WORDS_IN_PHRASE + 1 ;
	}
	//Adding ends.
*/
	if (  ((UINT)(StringRect.left - g_TotalWordRect.right) <= (UINT)SEPERATOR)
		&&(abs((int)(StringRect.bottom - g_TotalWordRect.bottom)) <= SEPERATOR))
	{
		//Added by XGL, see GetCurWordEnd for reasons. Nov 13th, 1998.
		if ((BYTE)(szBuff[nBegin]) >= 0xa1 
			&& (BYTE)(szBuff[nBegin]) <= 0xa9)
			
		{
			return;
		}
		//Adding ends. XGL, Nov 13th, 1998.
		//Added by XGL, Sep 17th, 1998
		if (g_bMouseInTotalWord && g_bPhraseNeeded
			&& (g_nWordsInPhrase > MIN_WORDS_IN_PHRASE)
			&& (g_nWordsInPhrase < MAX_WORDS_IN_PHRASE)
			&& (szBuff[nBegin] == ' '))
		{
			//other chars between nBegin and nEnd must be space
			nPos = nBegin ;
			while (szBuff[nPos] == ' ' && nPos <= nEnd)
			{
				nPos++ ;
			}      
			//Modified by XGL, Nov 11th, 1998  
			//We allow just one space between words of a phrase
			//if (nPos <= nEnd)      
			if (nPos <= nEnd || nPos - nBegin > 1)			
			{
				//stop get word
				g_nWordsInPhrase = MAX_WORDS_IN_PHRASE + 1 ;
			}
			else
			{
				g_nWordsInPhrase++ ;
			}
			
		}

		if (g_nWordsInPhrase >= MAX_WORDS_IN_PHRASE)
		{
			//enough words, no need to do more work
			return ;
		}
		//Adding ends.

		if ((g_nCurCaretPlace != -1)&&(g_nCurCaretPlaceInTotalWord == -1))
		{
			g_nCurCaretPlaceInTotalWord = strlen(g_szTotalWord) + g_nCurCaretPlace;
		}
		CopyWord(g_szTotalWord + strlen(g_szTotalWord), szBuff, nBegin, nEnd);
		g_TotalWordRect.right  = StringRect.right;

		//Added by XGL, Sep 29th, 1998
		if (!strchr(g_szTotalWord, ' ') && (*(szBuff + nBegin) != ' '))
		{
			g_rcFirstWordRect.right  = StringRect.right;
		}
		//Adding ends.

		if (bInCurWord)
		{
			g_bMouseInTotalWord = TRUE;
			//Added by XGL, Sep 17th, 1998
			//When the word pointed to by mouse was got,
			//attain it's char-type.        
			
			g_nPhraseCharType = nCharType ;
			g_nWordsInPhrase++ ;
			
			//Adding ends.
			
			//Added by XGL, Sep 29th, 1998
			g_rcFirstWordRect.left   = StringRect.left;
			g_rcFirstWordRect.right  = StringRect.right;
			g_rcFirstWordRect.top    = StringRect.top;
			g_rcFirstWordRect.bottom = StringRect.bottom;
			//Adding ends.
		}
	}
	else
	{
		if (!g_bMouseInTotalWord)
		{
			CopyWord(g_szTotalWord, szBuff, nBegin, nEnd);
			g_TotalWordRect.left   = StringRect.left;
			g_TotalWordRect.right  = StringRect.right;
			g_TotalWordRect.top    = StringRect.top;
			g_TotalWordRect.bottom = StringRect.bottom;
			g_CharType = nCharType;
			g_bMouseInTotalWord = FALSE;
			g_nCurCaretPlaceInTotalWord = -1;
			if (g_nCurCaretPlace != -1)
			{
				g_nCurCaretPlaceInTotalWord = g_nCurCaretPlace;
			}
			if (bInCurWord)
			{
				g_bMouseInTotalWord = TRUE;
				//Added by XGL, Sep 17th, 1998
				//When the word pointed to by mouse was got,
				//attain it's char-type.
				g_nPhraseCharType = nCharType ;
				g_nWordsInPhrase++ ;
				//Adding ends.
				//Added by XGL, Sep 29th, 1998
				g_rcFirstWordRect.left   = StringRect.left;
				g_rcFirstWordRect.right  = StringRect.right;
				g_rcFirstWordRect.top    = StringRect.top;
				g_rcFirstWordRect.bottom = StringRect.bottom;
				//Adding ends.
			}
		}
	}
}

////////////////////////////////////////////////////////////////////
// Deal with memdc string.

void AddToTextOutBuffer(HDC hMemDC, LPSTR szBuff, int cbLen, int x, int y, int FAR* lpDx)
{
	int  nPrevWord, nCurrentWord, CharType;
	RECT PrevWordRect, NextWordRect;
	int  nLen, i;
	
	if (cbLen >= (int)(BUFFERLENGTH - strlen(szMemDCWordBuff) - 2))
	{
		// buffer too small.
		return;
	}

	// Write string to buffer.
	nLen = strlen(szMemDCWordBuff);
	strncpy(szMemDCWordBuff + nLen, szBuff, cbLen);
	szMemDCWordBuff[nLen + cbLen    ] = ' ';
	szMemDCWordBuff[nLen + cbLen + 1] = 0x00;

	nPrevWord = nCurrentWord = -1;
	while (nCurrentWord < cbLen)
	{
//////////////////////////////////////////////////////////////
// Modified by Yan/gang 1998.02.20
// Reason: 
//         It cause the system destroy when the user run "WBLASTER"
//         The reason is the the array is overflow.
//         The WordBuffer array size is MEMDC_MAXNUM.
//         But it's point (nWordNum) is large than MEMDC_MAXNUM,
//         also, nWordNum is small than MAXNUM.
//		if (nWordNum >= MAXNUM)
		if (nWordNum >= MEMDC_MAXNUM)
			break;
// Modify end.
//////////////////////////////////////////////////////////////

		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);

		GetStringRect(hMemDC, szBuff, nPrevWord + 1, x, y, &PrevWordRect, lpDx);
		GetStringRect(hMemDC, szBuff, nCurrentWord + 1 , x, y, &NextWordRect, lpDx);
		
		WordBuffer[nWordNum].nBegin = nLen + nPrevWord + 1;
		WordBuffer[nWordNum].nEnd   = nLen + nCurrentWord;
		WordBuffer[nWordNum].hMemDC = hMemDC;
		WordBuffer[nWordNum].CharType = CharType;
		WordBuffer[nWordNum].wordRect.left   = PrevWordRect.right;
		WordBuffer[nWordNum].wordRect.right  = NextWordRect.right;
		WordBuffer[nWordNum].wordRect.top    = NextWordRect.top;
		WordBuffer[nWordNum].wordRect.bottom = NextWordRect.bottom;
		
		nWordNum++;

		if (nCurrentWord >= cbLen - 1)
			break;
	}
	
	GetStringLeftRight(hMemDC, szBuff, 0, x, &PrevWordRect, lpDx);
	for (i = 0; i < cbLen; i++)
	{
		GetStringLeftRight(hMemDC, szBuff, i+1, x, &NextWordRect, lpDx);
		pnMemDCCharLeft[nLen + i]  = PrevWordRect.right;
		pnMemDCCharRight[nLen + i] = NextWordRect.right;
	
		CopyRect(&PrevWordRect, &NextWordRect);
	}
}

void GetMemWordStringRect(int nWordCode, int nOffset, LPRECT lpStringRect)
{
	POINT  WndPos;
	int    nNum;

	if (nWordCode >= nWordNum)
	{
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		
		return;
	}
	
	CopyRect(lpStringRect, &(WordBuffer[nWordCode].wordRect));
	if (nOffset != MEMDC_TOTALWORD)
	{
		nNum = WordBuffer[nWordCode].nBegin + nOffset;
		lpStringRect->left = pnMemDCCharLeft[nNum];
//		lpStringRect->left = pnMemDCCharRight[nNum];
		lpStringRect->right = pnMemDCCharRight[nNum];
	}
	
	WndPos.x = LOWORD(g_dwDCOrg);
	WndPos.y = HIWORD(g_dwDCOrg);
	
	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

void CheckMemDCWordBuffer(HDC hdcdest, HDC hdcSrc)
{
	int i;
	
	for (i = 0; i < nWordNum; i++)
	{
		if (WordBuffer[i].hMemDC == hdcSrc)
		{
			CheckMouseInMemDCWord(i);
		}
		else
		{
#ifdef _DICTING_
			if (CheckDCWndClassName(hdcdest))
			{
				CheckMouseInMemDCWord(i);
			}
#endif
		}
//		CheckMouseInMemDCWord(i);
	}                               
}

#ifdef _DICTING_

#define IE4_CLIENT_CLASSNAME		"Internet Explorer_Server"// 定义取词中 DC 所属窗口的类名。
#define OUTLOOK_EDIT_CLASSNAME		"RichEdit20A"
#define MAX_CLASS_NAME 256

extern WINDOWFROMDC_PROC WindowFromDC;
int  g_nWorkOnClassNum = 2;
char g_szWorkOnClassName[2][MAX_CLASS_NAME] = { 
									IE4_CLIENT_CLASSNAME,	// IE 4.0
									OUTLOOK_EDIT_CLASSNAME	// OutLook
								};

_inline BOOL CheckDCWndClassName(HDC hDC)
{
	HWND hWndFromDC;
	char szClassName[MAX_CLASS_NAME];
	int i;

	hWndFromDC = WindowFromDC(hDC);
	GetClassName(hWndFromDC, szClassName, MAX_CLASS_NAME);

	for (i = 0; i < g_nWorkOnClassNum; i++)
	{
		if (lstrcmp(szClassName, (LPSTR)g_szWorkOnClassName[i]) == 0)
		{
			return TRUE;
		}
    }
	return FALSE;
}

#endif

__inline DWORD CheckMouseInMemDCWord(int nWordCode)
{
	RECT  StringRect;

	GetMemWordStringRect(nWordCode, MEMDC_TOTALWORD, &StringRect);

//	if (PtInRect(&StringRect, g_CurMousePos))
	if (  (StringRect.left   <= g_CurMousePos.x)
		&&(StringRect.right  >= g_CurMousePos.x)
		&&(StringRect.top    <= g_CurMousePos.y)
		&&(StringRect.bottom >= g_CurMousePos.y))
	{
		switch (WordBuffer[nWordCode].CharType)
		{
			case CHAR_TYPE_HZ:
			case CHAR_TYPE_ASCII:
				 CopyWord(g_szCurWord, szMemDCWordBuff, WordBuffer[nWordCode].nBegin, WordBuffer[nWordCode].nEnd);
				 g_CurWordRect.left   = StringRect.left;
				 g_CurWordRect.right  = StringRect.right;
				 g_CurWordRect.top    = StringRect.top;
				 g_CurWordRect.bottom = StringRect.bottom;
				 
				 g_nCurCaretPlace = -1;
				 CalculateCaretPlaceInMemDCWord(nWordCode);
				 
				 break;
			case CHAR_TYPE_OTHER:
				 break;
		}
		AddToTotalWord(szMemDCWordBuff, 
						0,  // Ignor
						WordBuffer[nWordCode].nBegin, 
						WordBuffer[nWordCode].nEnd, 
						WordBuffer[nWordCode].CharType, 
						StringRect, 
						TRUE);

		if (  (WordBuffer[nWordCode].CharType == CHAR_TYPE_OTHER)
		    &&(g_CurMousePos.x == StringRect.right))
		{
			return NO_CURMOUSEWORD;
		}
		return HAS_CURMOUSEWORD;
	}
	else
	{
	}

	AddToTotalWord(szMemDCWordBuff, 
				   0,  // Ignor
				   WordBuffer[nWordCode].nBegin, 
				   WordBuffer[nWordCode].nEnd, 
				   WordBuffer[nWordCode].CharType, 
				   StringRect, 
				   FALSE);

	return NO_CURMOUSEWORD;   
}

// 用於计算指定位置在字串中的位置·
__inline DWORD CalculateCaretPlaceInMemDCWord(int nWordCode)
{
	RECT  StringRect;
	int   i;

	if (  (WordBuffer[nWordCode].CharType == CHAR_TYPE_HZ) 
	    &&(WordBuffer[nWordCode].nBegin    == WordBuffer[nWordCode].nEnd))
	{
		g_nCurCaretPlace = -1;
		return 0L;
	}

	GetMemWordStringRect(nWordCode, MEMDC_TOTALWORD, &StringRect);
	
	if (CalcCaretInThisPlace(g_CurMousePos.x, StringRect.left))
	{
//		g_nCurCaretPlace = WordBuffer[nWordCode].nBegin;
		g_nCurCaretPlace = 0;
		return 0l;
	}

	if (CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right))
	{
//		g_nCurCaretPlace = WordBuffer[nWordCode].nEnd + 1;
		g_nCurCaretPlace = WordBuffer[nWordCode].nEnd - WordBuffer[nWordCode].nBegin + 1;
		return 0l;
	}
	
	for (i = WordBuffer[nWordCode].nBegin; i <= WordBuffer[nWordCode].nEnd; i++)
	{
		GetMemWordStringRect(nWordCode, i - WordBuffer[nWordCode].nBegin, &StringRect);
		if (CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right))
		{
//			g_nCurCaretPlace = i + 1;
			g_nCurCaretPlace = i - WordBuffer[nWordCode].nBegin + 1;
			return 0l;
		}
	}
	
	return 0L;
}

/*
		if (nCurCharNum != nFromPlace)
		{
#ifdef _DEBUG
			DbgPrintf("NhWSrh.DLL          nCurCharNum != nFromPlace");
#endif
			return TRUE;
		}

	if (  (nCurCharNum == nLength - 1)
		&&(lpString[nCurCharNum] == CHAR_LINK)
		   ||(lpString[nCurCharNum] == '?')
		   ||(lpString[nCurCharNum] == '*'))
	{
		return FALSE;
	}

	if (  (nCurCharNum != nFromPlace)
		&&   (lpString[nCurCharNum] == CHAR_LINK)
		   ||(lpString[nCurCharNum] == '?')
		   ||(lpString[nCurCharNum] == '*'))
	{
#ifdef _DEBUG
		DbgPrintf("NhWSrh.DLL          3");
#endif
		return TRUE;
	}
	
	if ((nCurCharNum == '?') || (nCurCharNum == '*'))
	{
#ifdef _DEBUG
		DbgPrintf("NhWSrh.DLL          *?");
#endif
		if (nCurCharNum == nLength - 1)
		{
#ifdef _DEBUG
			DbgPrintf("NhWSrh.DLL          nCurCharNum == nLength - 1");
#endif
			return FALSE;
		}
		if (GetCharType(lpString[nCurCharNum + 1]) != CHAR_TYPE_ASCII)
		{
#ifdef _DEBUG
			DbgPrintf("NhWSrh.DLL          etCharType(lpString[nCurCharNum + 1]) != CHAR_TYPE_ASCII");
#endif
			return FALSE;
		}
		if (nCurCharNum == nFromPlace)
		{
#ifdef _DEBUG
			DbgPrintf("NhWSrh.DLL          nCurCharNum == nFromPlace");
#endif
			return FALSE;
		}
			
		return TRUE;
	}
__inline int  FindAWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	int j;
	while (i < nLength)
	{
		if (  (lpString[i] == '?')
		    ||(lpString[i] == '*'))
		{
#ifdef _DEBUG
				DbgPrintf("NhWSrh.DLL          ?*");
#endif
			if (i == nLength - 1)
			{
#ifdef _DEBUG
				DbgPrintf("NhWSrh.DLL          i == nLength - 1");
#endif
//				return i - 1;
				return i;
			}
			j = i;
			while(j < nLength)
			{
				if ((lpString[j] != '?')&&(lpString[j] != '*'))
				{
#ifdef _DEBUG
				DbgPrintf("NhWSrh.DLL          is not ?*");
#endif
					break;
				}
				j++;
			}
			
			if (j == nLength)
				return j-1;
			
			if (IsASCIIWord(lpString, nFromPlace, nLength, j))
			{
				i = j;
			}
			else
			{
//				return i - 1;
				return j - 1;
			}
		}
		// 用於处理 '-' 的几种情况·
		if (IsASCIIWord(lpString, nFromPlace, nLength, i))
//		if (GetCharType(lpString[i]) == CHAR_TYPE_ASCII)
		{
			i++;
		}
		else
		{
			return i-1;
		}
	} 
	
	return nLength - 1;
}

__inline BOOL IsASCIIWord(LPCSTR lpString, int nFromPlace, int nLength, int nCurCharNum)
{
	if (GetCharType(lpString[nCurCharNum]) == CHAR_TYPE_ASCII)
	{
		return TRUE;
	}
	
	if (lpString[nCurCharNum] == CHAR_LINK)
	{
		if (nCurCharNum == nLength - 1)
		{
			return FALSE;
		}
		if (GetCharType(lpString[nCurCharNum + 1]) == CHAR_TYPE_ASCII)
		{
			return TRUE;
		}
	}

/*	if (  (lpString[nCurCharNum] == '?')
		||(lpString[nCurCharNum] == '*'))
	{
		if (nCurCharNum == nLength - 1)
		{
			return FALSE;
		}
		nCurCharNum++;
		while(nCurCharNum < nLength)
		{
			if (GetCharType(lpString[nCurCharNum]) ==
		}
//		if (GetCharType(lpString[nCurCharNum + 1]) == CHAR_TYPE_ASCII)
//		{
//			return TRUE;
//		}
	}*/
//	return FALSE;
//}
//*/	

/*

__inline int  FindAWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
//	int j;
	while (i < nLength)
	{
		if (lpString[i] == CHAR_LINK)
		{
			if (IsASCIIWord(lpString, nFromPlace, nLength, i + 1))
			{
				i++;
			}
			else
			{
				return i-1;
			}
		}
		else
		{
			// 用於处理 '-' 的几种情况·
			if (IsASCIIWord(lpString, nFromPlace, nLength, i))
	//		if (GetCharType(lpString[i]) == CHAR_TYPE_ASCII)
			{
				i++;
			}
			else
			{
				return i-1;
			}
		}
	} 
	
	return nLength - 1;
}


__inline BOOL IsASCIIWord(LPCSTR lpString, int nFromPlace, int nLength, int nCurCharNum)
{
	if (GetCharType(lpString[nCurCharNum]) == CHAR_TYPE_ASCII)
	{
		return TRUE;
	}
    
	if (lpString[nCurCharNum] == CHAR_WILDCHAR1)
	{
		return TRUE;
	}

	if (lpString[nCurCharNum] == CHAR_WILDCHAR2)
	{
		return TRUE;
	}
//	if (lpString[nCurCharNum] == CHAR_LINK)
//	{
//		if (nCurCharNum == nLength - 1)
//		{
//			return FALSE;
//		}
//		if (GetCharType(lpString[nCurCharNum + 1]) == CHAR_TYPE_ASCII)
//		{
//			return TRUE;
//		}
//	}

	return FALSE;
}

*/