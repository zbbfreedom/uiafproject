
#include <windows.h>
#include "findword.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "..\pub3216.h"
#include <malloc.h>
#include "import.h"

extern char  g_szCurWord[];
extern RECT  g_CurWordRect;
extern int   g_nCurCaretPlace;

extern BOOL  g_bAllowGetCurWord;
extern POINT g_CurMousePos;
extern HWND  g_hNotifyWnd;

extern UINT         g_nTextAlign;
extern POINT        g_dwDCOrg;
extern int          g_nExtra;
extern POINT        g_CurPos;
extern TEXTMETRIC   g_tm;

#define CHAR_WILDCHAR1 '*'
#define CHAR_WILDCHAR2 '?'

///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// 用於修改在计算 TA_CENTER 情况的失误。
extern BOOL bRecAllRect;
extern RECT g_rcTotalRect;
// End Modify
///////////////////////////////////////////////////////////////////////////

#ifdef _DICTING_
/////////////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/19
// 主要解决通配符的问题。
extern int g_nGetWordStyle;
// End Add.
/////////////////////////////////////////////////////////////////////////////////
#endif

#define RIGHT  1
#define LEFT  -1

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
	if (g_nGetWordStyle == GETWORD_TW_ENABLE)
	{
		return FindTWWord(lpString, nFromPlace, nLength);
	}
	else
	{
		return FindDWord(lpString, nFromPlace, nLength);
	}

	return 0;
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

	return FALSE;
}
// End Add.
/////////////////////////////////////////////////////////////////////////////////
#else
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
    
	if (lpString[nCurCharNum] == CHAR_WILDCHAR1)
	{
		return TRUE;
	}

	if (lpString[nCurCharNum] == CHAR_WILDCHAR2)
	{
		return TRUE;
	}
	return FALSE;
}
#endif

__inline int  FindHZWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_HZ)
		{
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

	WndPos.y = g_dwDCOrg.y;

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
	WndPos.x = g_dwDCOrg.x;
	
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
///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/21
// 用於修改在计算 TA_RIGHT 情况的失误。
//			 lpStringRect->right = x;
//			 lpStringRect->left  = x - StringSize.cx;
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
// End Modify
///////////////////////////////////////////////////////////////////////////
//			 lpStringRect->right = x;
//			 lpStringRect->left  = x - StringSize.cx;
			 break;
		case TA_CENTER:
///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// 用於修改在计算 TA_CENTER 情况的失误。
//			 lpStringRect->right = x + StringSize.cx / 2;
//			 lpStringRect->left  = x - StringSize.cx / 2;
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
// End Modify
///////////////////////////////////////////////////////////////////////////
//			 lpStringRect->right = x + StringSize.cx / 2;
//			 lpStringRect->left  = x - StringSize.cx / 2;
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
	WndPos.x = g_dwDCOrg.x;
	WndPos.y = g_dwDCOrg.y;
	
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
///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/21
// 用於修改在计算 TA_RIGHT 情况的失误。
//			 lpStringRect->right = x;
//			 lpStringRect->left  = x - StringSize.cx;
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
// End Modify
///////////////////////////////////////////////////////////////////////////
//			 lpStringRect->right = x;
//			 lpStringRect->left  = x - StringSize.cx;
			 break;
		case TA_CENTER:
///////////////////////////////////////////////////////////////////////////
// Modify by Yan/Gang 1997/11/18
// 用於修改在计算 TA_CENTER 情况的失误。
//			 lpStringRect->right = x + StringSize.cx / 2;
//			 lpStringRect->left  = x - StringSize.cx / 2;
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
// End Modify
///////////////////////////////////////////////////////////////////////////
//			 lpStringRect->right = x + StringSize.cx / 2;
//			 lpStringRect->left  = x - StringSize.cx / 2;
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

		if (dwReturn == HAS_CURMOUSEWORD)
			return HAS_CURMOUSEWORD;
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
		Nh16_AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, &StringRect, TRUE, g_nCurCaretPlace);
		if (  (nCharType == CHAR_TYPE_OTHER)
		    &&(CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right)))
		{
			return NO_CURMOUSEWORD;
		}
		return HAS_CURMOUSEWORD;
	}
	else
	{
	}					

	Nh16_AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, &StringRect, FALSE, g_nCurCaretPlace);

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
	int   itemWidth; 
	int   TempPlace;

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
			 // 由於汉字都是等宽的字符，因此对於汉字的计算是通过计算
			 // 平均宽度来实现的·
			 itemWidth = (StringRect.right - StringRect.left) / (nEnd - nBegin + 1);
			 g_nCurCaretPlace = (g_CurMousePos.x - StringRect.left) * (nEnd - nBegin + 1) / (StringRect.right - StringRect.left);
             break;
             
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
			 itemWidth = (StringRect.right - StringRect.left) / (nEnd - nBegin + 1);
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

BOOL CalcCaretInThisPlace(int CaretX, int nPlace)
{
	if (CaretX == nPlace)
	{
		return TRUE;
	}
	
	if (CaretX == nPlace - 1)
	{
		return TRUE;
	}
	
	if (CaretX == nPlace + 1)
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

	itemWidth = (lpStringRect->right - lpStringRect->left) / (nEnd - nBegin + 1);

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


////////////////////////////////////////////////////////////////////
// Deal with memdc string.

void AddToTextOutBuffer(HDC hMemDC, LPSTR szBuff, int cbLen, int x, int y, int FAR* lpDx)
{
	int  nPrevWord, nCurrentWord, CharType;
	RECT PrevWordRect, NextWordRect;
	int  nLen, i;
	
	nLen = Nh16_GetTempLen();
	if (!Nh16_AddToTempBuff(szBuff, cbLen))
	{
		return;
	}

	nPrevWord = nCurrentWord = -1;
	while (nCurrentWord < cbLen)
	{
		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);

		GetStringRect(hMemDC, szBuff, nPrevWord + 1, x, y, &PrevWordRect, lpDx);
		GetStringRect(hMemDC, szBuff, nCurrentWord + 1 , x, y, &NextWordRect, lpDx);
		
		NextWordRect.left = PrevWordRect.right;
		if (!Nh16_AddToWordStruct(nLen + nPrevWord + 1, nLen + nCurrentWord, hMemDC, CharType, &NextWordRect))
		{
			break;
		}

		if (nCurrentWord >= cbLen - 1)
			break;
	}
	
	GetStringLeftRight(hMemDC, szBuff, 0, x, &PrevWordRect, lpDx);
	for (i = 0; i < cbLen; i++)
	{
		GetStringLeftRight(hMemDC, szBuff, i+1, x, &NextWordRect, lpDx);
		Nh16_AddToCharStruct(nLen + i, PrevWordRect.right, NextWordRect.right);
		CopyRect(&PrevWordRect, &NextWordRect);
	}
}

