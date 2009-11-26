/////////////////////////////////////////////////////////////////////////
//
// findword.c
//
// Date   : 04/18/99
//
/////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "stdio.h"
#include "string.h"
#include "findword.h"
#include "exports.h"
#include "public.h"
#include "dbgprint.h"

extern UINT BL_HASSTRING;

extern char  g_szTotalWord[BUFFERLENGTH];
extern RECT  g_TotalWordRect;
extern int   g_CharType;
extern int   g_nCurCaretPlaceInTotalWord;
extern int   g_bMouseInTotalWord;

extern int g_nWordsInPhrase ;
extern BOOL g_bPhraseNeeded ;
extern int g_nPhraseCharType ;

extern char szMemDCWordBuff[BUFFERLENGTH];
extern int  pnMemDCCharLeft[BUFFERLENGTH];
extern int  pnMemDCCharRight[BUFFERLENGTH];
extern WORDPARA WordBuffer[MEMDC_MAXNUM];
extern int nWordNum;

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

extern BOOL bRecAllRect;
extern RECT g_rcTotalRect;
extern RECT g_rcFirstWordRect;

#define RIGHT  1
#define LEFT  -1

#define IE4_CLIENT_CLASSNAME		"Internet Explorer_Server"// ﹚竡迭い DC ┮妮怠摸
#define OUTLOOK_EDIT_CLASSNAME		"RichEdit20A"
#define MAX_CLASS_NAME 256

int  g_nWorkOnClassNum = 2;
char g_szWorkOnClassName[2][MAX_CLASS_NAME] = { 
									IE4_CLIENT_CLASSNAME,	// IE 4.0
									OUTLOOK_EDIT_CLASSNAME	// OutLook
								};

extern int g_nGetWordStyle;

/**************************************************************************
**************************************************************************/
//获取字符类型
__inline int  GetCharType(char ch)
{
	BYTE chitem = ch;

	if (ch < 0)
		return CHAR_TYPE_HZ;//比较，如果小于0，认定汉字

	if (((ch >= 'a')&&(ch <= 'z'))||
	    ((ch >= 'A')&&(ch <= 'Z')))
	{
		return CHAR_TYPE_ASCII;//认定英文字符
	}
	
	return CHAR_TYPE_OTHER;	
}

__inline int  FindAWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	//Modified by ZHHN on 2000.4
	int nResult = 0;

	switch(g_nGetWordStyle)//判断g_nGetWordStyle,这个变量是在Exports.c里面定义的
	{
	//根据以下条件进行字符处理
	case GETWORD_D_ENABLE:
	case GETWORD_TW_ENABLE:
	case GETPHRASE_ENABLE:
	case GETWORD_D_TYPING_ENABLE:
		nResult = FindTWWord(lpString, nFromPlace, nLength);	//get word with '-'
		break;
	default:
		nResult = FindDWord(lpString, nFromPlace, nLength);		//get word without '-'
		break;
	}

	return nResult;
	//Modified end
}
//为了获得lpString里连续英文字符的最后一个位置，nFromPlace是起始位置，nLength是解析长度。
__inline int  FindDWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_ASCII)
		{
			i++;//如果是英文字符，当前位置+1	
		}
		else
		{
			return i-1;非CHAR_TYPE_ASCII的话，当前位置-1并返回
		}
	} 
	
	return nLength - 1;
}
//这个和FindDWord类似，也是为了获取连续英文字符的最后一个位置，只不过常量字符串lpString为包含'-'的情况
__inline int  FindTWWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (lpString[i] == CHAR_LINK)//这里的判断有点疑惑，应该是'-'这个字符，即：如果当前位置是'-',从当前位置的下一个判断
		{
			//如果满足上面的条件，判断下一个字符是否是英文字符
			if (IsASCIIWord(lpString, nFromPlace, nLength, i + 1))
			{
				i++;//是则+1
			}
			else
			{
				return i-1;不是的话返回i-1
			}
		}
		else//对于当前的位置不是'-'字符的话，判断当前位置
		{
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
//判断是否是英文字符，实际就调用了GetCharType方法而已，感觉nFromPlace和nLength这两个参数多余
__inline BOOL IsASCIIWord(LPCSTR lpString, int nFromPlace, int nLength, int nCurCharNum)
{
	if (GetCharType(lpString[nCurCharNum]) == CHAR_TYPE_ASCII)
	{
		return TRUE;
	}

	return FALSE;
}
//获取连续中文字符的最后位置（与FindDWord功能类似）
__inline int  FindHZWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;//起始位置
	//判断是否为非法的字符：主要是把：全角空格，|,\等等字符过滤掉
	if ((BYTE)(lpString[nFromPlace]) >= 0xa1
		&& (BYTE)(lpString[nFromPlace]) <= 0xa9)
	{
		return nFromPlace + 1 ;
	}
	//在排除上面的情况下（此时起始位置为中文字符）
	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_HZ)
		{
			if ((BYTE)(lpString[i]) >= 0xa1
				&& (BYTE)(lpString[i]) <= 0xa9)//判断是否为特殊的符号，判断之后的处理逻辑与查找英文字符类似
			{
				return i - 1;
			}

			i = i + 2;
		}
		else
		{
			return i - 1;
		}
	} 
	
	return nLength - 1;
}
//获取下一个字符（包括英文或中文字符）的其实位置
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
//根据传入的字符类型参数，分别求连续的最后一个字符的位置
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
//拷贝制定部分的字符串，
__inline void CopyWord(LPSTR lpWord, LPCSTR lpString, int nBegin, int nEnd)
{
	int i;
	for ( i = nBegin; i <= nEnd; i++)
	{
		lpWord[i - nBegin] = lpString[i];
	}
	lpWord[nEnd - nBegin + 1] = '\0';//指定字符串结尾
}
//获取当前的绘制矩形的纵向位置（应该就是我们重绘的位置）
void GetStringTopBottom(HDC hDC, int y, RECT* lpStringRect)
{
	POINT  WndPos;

	WndPos.y = g_dwDCOrg.y;//注：g_dwDCOrg这个点也是在Exports.c中定义的，初始为{0,0}

    if (TA_UPDATECP & g_nTextAlign)//注g_nTextAlign是在Exports.c中定义的，初始化为0。//判断文本对齐模式是否为TA_UPDATECP以及g_nTextAlign
    {
    	y = g_CurPos.y;//在当前位置重绘，g_CurPos在Exports.c中定义
    }
    
	switch ((TA_TOP | TA_BOTTOM)&g_nTextAlign)//对不同的对齐方式获取
	{
		case TA_BOTTOM://底对齐：矩形框的纵向的top值为y-字符高度+字符高度的顶部空间（为什么这样求值不太清楚）
			 lpStringRect->top    = y - g_tm.tmHeight + g_tm.tmInternalLeading;
			 lpStringRect->bottom = y;
			 break;
		case TA_BASELINE:
			 lpStringRect->top    = y - g_tm.tmAscent;//y-字符上部高度
			 lpStringRect->bottom = y + g_tm.tmDescent;//y+字符下部高度
			 break;
		case TA_TOP://上对齐，处理逻辑和下对齐类似
		default:
			 lpStringRect->top    = y;
			 lpStringRect->bottom = y + g_tm.tmHeight + g_tm.tmInternalLeading;
			 break;
	}
	
	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);//对上面求的的逻辑坐标进行转换

	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
}
//获取绘制矩形的横向位置。
void GetStringLeftRight(HDC hDC, LPSTR szBuff, int cbLen, int x, RECT* lpStringRect, CONST INT *lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    int i;

	if (cbLen < 0)//如果参数cbLen<0，矩形结构的四个参数都为0
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}
	//获取指定长度字符串的高度和宽度，StringSize存储
	GetTextExtentPoint32(hDC, szBuff, cbLen, &StringSize);	//Modified by ZHHN on 2000.1.14

	WndPos.x = g_dwDCOrg.x;
	
	if (lpDx != NULL)//lpDx!=null说明绘制字符串的时候设置了字符间距
	{
		StringSize.cx = 0;
		for (i = 0; i < cbLen; i++)
		{
			StringSize.cx += lpDx[i];//指定字符间距不为空，那么指定长度的字符串的宽度需要加上字符间距
		}
	}

    if (TA_UPDATECP & g_nTextAlign)//当前位置绘制
    {
    	x = g_CurPos.x;
    }
    //下面的switch语句的处理逻辑与GetStringTopBottom类似了，分别针对不同的对齐方式，指定矩形框的位置
	switch ((TA_LEFT | TA_CENTER | TA_RIGHT)&g_nTextAlign)
	{
		case TA_RIGHT://右对齐
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
		case TA_CENTER://居中对齐
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
	
	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);//转换逻辑坐标

	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

//Created due to fixing 
// Bug4: get big5 chinese word's position error from outlook98 in Windows 2000 
// Bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000
//获取中文字符的处理方法，跟英文字符的处理逻辑是一样的，有肯能就是在存储英文字符与中文字符之间有差异，（看他的注释也是，存中文字符是抛异常应该是）
//所以下面的lpWideCharStr声明为LPCWSTR类型的，为16位，而上面处理英文字符时是8位的数据类型。
void GetStringLeftRightW(HDC hDC, LPCWSTR lpWideCharStr, UINT cbWideChars, int x, RECT* lpStringRect, CONST INT *lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    UINT i;

	if (cbWideChars < 0)
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}
	
	WndPos.x = g_dwDCOrg.x;
	
	GetTextExtentPoint32W(hDC, lpWideCharStr, cbWideChars, &StringSize);

	if (lpDx != NULL)
	{
		StringSize.cx = 0;
		for (i = 0; i < cbWideChars; i++)
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
//获取整个矩形的横向和纵向的位置，实际上就是：GetStringLeftRight+GetStringTopBottom的合。但是不知道为什么，他不是调用上面的方法，而是又重新写了一遍，就是把
//前面的两个方法内容copy了一遍而已
void GetStringRect(HDC hDC, LPSTR szBuff, int cbLen, int x, int y, RECT* lpStringRect, CONST INT *lpDx)
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

	GetTextExtentPoint32(hDC, szBuff, cbLen, &StringSize);

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
	//获取横向位置
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
	//获取纵向位置
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

//Created due to fixing 
// Bug4: get big5 chinese word's position error from outlook98 in Windows 2000 
// Bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000
//为中文字符写的获取矩形的位置，与英文字符逻辑一样，只不过传参的类型由8位改为16位
void GetStringRectW(HDC hDC, LPCWSTR lpWideCharStr, UINT cbWideChars, int x, int y, RECT* lpStringRect, CONST INT *lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    UINT i;

	if (cbWideChars < 0)
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}

	WndPos.x = g_dwDCOrg.x;
	WndPos.y = g_dwDCOrg.y;

	GetTextExtentPoint32W(hDC, lpWideCharStr, cbWideChars, &StringSize);
	
	if (lpDx != NULL)
	{
		StringSize.cx = 0;
		for (i = 0; i < cbWideChars; i++)
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
//获取当前鼠标下的字
DWORD GetCurMousePosWord(HDC   hDC, 
						 LPSTR szBuff, 
						 int   cbLen, 
						 int   x, 
						 int   y, 
						 CONST INT *lpDx)
{
	int   nCurrentWord, nPrevWord;
	RECT  StringRect;
	int   CharType;
	int   nLeft;
	DWORD dwReturn;

	DWORD dwResult = NO_CURMOUSEWORD;	//Added by ZHHN on 2000.4
/*
	if (cbLen != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "1........... GetCurMousePosWord : (%s) (%d) %d %d\n", szBuff, cbLen, x, y);
		OutputDebugString(cBuffer);
	}
*/
	//矩形的纵向位置
	GetStringTopBottom(hDC, y, &StringRect);

	//矩形的纵向位置与当前鼠标位置进行判断，但是判断逻辑有点不明白（感觉弄反了。。）
	if ((StringRect.top > g_CurMousePos.y) || (StringRect.bottom < g_CurMousePos.y))
	{
		return NO_CURMOUSEWORD;
	}
	//调用前面获取矩形位置的方法
	GetStringRect(hDC, szBuff, cbLen, x, y, &StringRect, lpDx);
	nLeft = StringRect.left;

	nPrevWord = nCurrentWord = -1;//当前字和前一字都记为-1
	while (nCurrentWord < cbLen)
	{
		CharType     = GetCharType(szBuff[nCurrentWord + 1]);//先获取下一字的类型
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);//获取连续类型相同的字符的最后一个位置
		dwReturn     = CheckMouseInCurWord(hDC, szBuff, cbLen, x, y, lpDx, &nLeft, nPrevWord + 1, nCurrentWord, CharType);
/*
		if (cbLen != 0)
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "2........... GetCurMousePosWord : %s %d %s %08x\n", 
				szBuff, cbLen, "HAS_CURMOUSEWORD", g_hNotifyWnd);
			OutputDebugString(cBuffer);
		}
*/
		if (dwReturn == HAS_CURMOUSEWORD)
		{
			if (CharType == CHAR_TYPE_OTHER)
			{
				PostMessage(g_hNotifyWnd, BL_HASSTRING, 0, 0l);				
			}
		}
		else
		{
			if (g_bMouseInTotalWord)
			{
				PostMessage(g_hNotifyWnd, BL_HASSTRING, 0, 0l);
			}
		}

		if (dwReturn == HAS_CURMOUSEWORD)
		{
			//return HAS_CURMOUSEWORD;
			dwResult = HAS_CURMOUSEWORD;//Modified by ZHHN on 2000.4 in order to get phrase
		}

		if (nCurrentWord >= cbLen - 1)
		{
			//return NO_CURMOUSEWORD;
			break;	//Modified by ZHHN on 2000.4
		}
	}
		
	//return NO_CURMOUSEWORD;
	return dwResult;	//Modified by ZHHN on 2000.4
}

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

DWORD GetCurMousePosWordW(HDC   hDC, 
						  LPCWSTR lpWideCharStr, 
						  UINT cbWideChars, 
						  int   x, 
						  int   y, 
						  CONST INT *lpDx)
{
	int   nCurrentWord, nPrevWord;
	RECT  StringRect;
	int   CharType;
	int   nLeft;
	DWORD dwReturn;

	DWORD dwResult = NO_CURMOUSEWORD;	//Added by ZHHN on 2000.4 in order to get phrase

	int cbLen;
	char szBuff[256];

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;

/*
	if (cbLen != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "1........... GetCurMousePosWord : (%s) (%d) %d %d\n", szBuff, cbLen, x, y);
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("GetCurMousePosWordW : szBuff(%s), cbLen(%d)\n", szBuff, cbLen);

	GetStringTopBottom(hDC, y, &StringRect);

	if ((StringRect.top > g_CurMousePos.y) || (StringRect.bottom < g_CurMousePos.y))
	{
		return NO_CURMOUSEWORD;
	}

	GetStringRectW(hDC, lpWideCharStr, cbWideChars, x, y, &StringRect, lpDx);
	nLeft = StringRect.left;

	nPrevWord = nCurrentWord = -1;
	while (nCurrentWord < cbLen)
	{
		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);
		dwReturn     = CheckMouseInCurWordW(hDC, lpWideCharStr, cbWideChars, x, y, lpDx, &nLeft, nPrevWord + 1, nCurrentWord, CharType);
/*
		if (cbLen != 0)
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "2........... GetCurMousePosWord : %s %d %s %08x\n", 
				szBuff, cbLen, "HAS_CURMOUSEWORD", g_hNotifyWnd);
			OutputDebugString(cBuffer);
		}
*/
		if (dwReturn == HAS_CURMOUSEWORD)
		{
			if (CharType == CHAR_TYPE_OTHER)
			{
				PostMessageW(g_hNotifyWnd, BL_HASSTRING, 0, 0l);				
			}
		}
		else
		{
			if (g_bMouseInTotalWord)
			{
				PostMessageW(g_hNotifyWnd, BL_HASSTRING, 0, 0l);				
			}
		}

		if (dwReturn == HAS_CURMOUSEWORD)
		{
			//return HAS_CURMOUSEWORD;
			dwResult = HAS_CURMOUSEWORD;	//Modified by ZHHN on 2000.4 in order to get phrase
		}

		if (nCurrentWord >= cbLen - 1)
		{
			//return NO_CURMOUSEWORD;
			break;	//Modified by ZHHN on 2000.4
		}
	}
		
	//return NO_CURMOUSEWORD;
	return dwResult;	//Modified by ZHHN on 2000.4
}
//当前鼠标是否聚焦在字符串上
DWORD CheckMouseInCurWord(HDC   hDC, 
						  LPSTR szBuff, 
						  int   cbLen, 
						  int   x, 
						  int   y, 
						  CONST INT *lpDx,
						  int*  lpLeft,
						  int   nBegin,    // = nPrevWord + 1
						  int   nEnd,
						  int   nCharType)
{
	RECT  StringRect;

	GetStringRect(hDC, szBuff, nEnd + 1, x, y, &StringRect, lpDx);
	StringRect.left = *lpLeft;
	*lpLeft = StringRect.right;
/*
	if (cbLen != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "........... CheckMouseInCurWord : %s %d (%d,%d) (%d,%d,%d,%d)\n", 
			"start", nCharType, g_CurMousePos.x, g_CurMousePos.y, 
			StringRect.left, StringRect.top, StringRect.right, StringRect.bottom);
		OutputDebugString(cBuffer);
	}
*/
	if (  ((g_CurMousePos.x >= StringRect.left    ) && (g_CurMousePos.x <= StringRect.right))//鼠标横坐标在字符串宽度之内
	    || (g_CurMousePos.x == StringRect.left - 1)//只有一个字的情况
	    || (g_CurMousePos.x == StringRect.right + 1))
	{
/*
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "........... CheckMouseInCurWord : %s %d\n", 
				"start", nCharType);
			OutputDebugString(cBuffer);
		}
*/
		switch (nCharType)
		{
			case CHAR_TYPE_HZ:
			case CHAR_TYPE_ASCII:
				 CopyWord(g_szCurWord, szBuff, nBegin, nEnd);//复制字符串
				 g_CurWordRect.left   = StringRect.left;//应该是制定重绘的范围
				 g_CurWordRect.right  = StringRect.right;
				 g_CurWordRect.top    = StringRect.top;
				 g_CurWordRect.bottom = StringRect.bottom;
/*				 
				{
					char cBuffer[0x100];
					wsprintf(cBuffer, "!!!........... CheckMouseInCurWord : %d %d %d %d\n", 
						g_CurWordRect.left, g_CurWordRect.top, g_CurWordRect.right, g_CurWordRect.bottom);
					OutputDebugString(cBuffer);
				}
*/
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
//				 g_bMouseInTotalWord = TRUE;
				 break;

			case CHAR_TYPE_OTHER:
				 break;
		}

		AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, TRUE);

		if (  (nCharType == CHAR_TYPE_OTHER)
		    &&(CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right)))
		{
			return HAS_CURMOUSEWORD;
		}

		return HAS_CURMOUSEWORD;
	}
	else
	{
//		g_bMouseInTotalWord = FALSE;
	}					

	AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, FALSE);

	return NO_CURMOUSEWORD;   
}

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

DWORD CheckMouseInCurWordW(HDC   hDC, 
						   LPCWSTR lpWideCharStr, 
						   UINT cbWideChars, 
						   int   x, 
						   int   y, 
						   CONST INT *lpDx,
						   int*  lpLeft,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   nCharType)
{
	RECT  StringRect;

	int cbLen;
	char szBuff[256];

	wchar_t lpTemp[256];
	int nTempLen;

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;

	nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, nEnd + 1, lpTemp, 256);
	GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &StringRect, lpDx);

	StringRect.left = *lpLeft;
	*lpLeft = StringRect.right;
/*
	if (cbLen != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "........... CheckMouseInCurWord : %s %d (%d,%d) (%d,%d,%d,%d)\n", 
			"start", nCharType, g_CurMousePos.x, g_CurMousePos.y, 
			StringRect.left, StringRect.top, StringRect.right, StringRect.bottom);
		OutputDebugString(cBuffer);
	}
*/
	if (  ((g_CurMousePos.x >= StringRect.left    ) && (g_CurMousePos.x <= StringRect.right))
	    || (g_CurMousePos.x == StringRect.left - 1)
	    || (g_CurMousePos.x == StringRect.right + 1))
	{
/*
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "........... CheckMouseInCurWord : %s %d\n", 
				"start", nCharType);
			OutputDebugString(cBuffer);
		}
*/
		switch (nCharType)
		{
			case CHAR_TYPE_HZ:
			case CHAR_TYPE_ASCII:
				 CopyWord(g_szCurWord, szBuff, nBegin, nEnd);
				 g_CurWordRect.left   = StringRect.left;
				 g_CurWordRect.right  = StringRect.right;
				 g_CurWordRect.top    = StringRect.top;
				 g_CurWordRect.bottom = StringRect.bottom;
/*				 
				{
					char cBuffer[0x100];
					wsprintf(cBuffer, "!!!........... CheckMouseInCurWord : %d %d %d %d\n", 
						g_CurWordRect.left, g_CurWordRect.top, g_CurWordRect.right, g_CurWordRect.bottom);
					OutputDebugString(cBuffer);
				}
*/
				 g_nCurCaretPlace = -1;
				 CalculateCaretPlaceW(hDC, 
									 lpWideCharStr, 
									 cbWideChars,
									 x,
									 y,
									 lpDx,
									 nBegin,
									 nEnd,
									 nCharType);
//				 g_bMouseInTotalWord = TRUE;
				 break;

			case CHAR_TYPE_OTHER:
				 break;
		}

		AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, TRUE);

		if (  (nCharType == CHAR_TYPE_OTHER)
		    &&(CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right)))
		{
			return HAS_CURMOUSEWORD;
		}

		return HAS_CURMOUSEWORD;
	}
	else
	{
//		g_bMouseInTotalWord = FALSE;
	}					

	AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, FALSE);

	return NO_CURMOUSEWORD;   
}
//鼠标聚焦的矩形块的位置
DWORD CalculateCaretPlace(HDC   hDC, 
						  LPSTR szBuff, 
						  int   cbLen, 
						  int   x, 
						  int   y, 
						  CONST INT *lpDx,
						  int   nBegin,    // = nPrevWord + 1
						  int   nEnd,
						  int   nCharType)
{
	RECT  StringRect, BeginRect;
	RECT  CaretPrevRect, CaretNextRect;
	double itemWidth; 
	int   TempPlace;
	int   i;

	if ((nCharType == CHAR_TYPE_HZ) && (nBegin == nEnd))//中文字符，且字符的
	{
		g_nCurCaretPlace = -1;
		return 0L;
	}
	//下面是获取相邻的两个矩形的位置（区分矩形是根据字符类型）
	GetStringRect(hDC, szBuff, nBegin, x, y, &BeginRect, lpDx);
	
	GetStringRect(hDC, szBuff, nEnd + 1,   x, y, &StringRect, lpDx);
	StringRect.left = BeginRect.right;//这里赋值的意义不太清楚，
    if (StringRect.left == StringRect.right)
    {
		g_nCurCaretPlace = -1;
		return 0L;
    }
	
	switch (nCharType)
	{
		case CHAR_TYPE_HZ:
			 itemWidth = ((double)StringRect.right - (double)StringRect.left + 1) / ((double)nEnd - (double)nBegin + 1);//平均每个字的宽度
			 for (i = 0; i <= (nEnd - nBegin + 1); i++)
			 {
			 	if (CalcCaretInThisPlace(g_CurMousePos.x, (double)((double)StringRect.left + (double)(itemWidth * i))))//计算鼠标聚焦的矩形块的宽度
			 	{
				 	g_nCurCaretPlace = i;
				 	i = nEnd - nBegin + 2;
			 	}
			 }
             break;
		case CHAR_TYPE_ASCII:
			 itemWidth = (StringRect.right - StringRect.left + 1) / (nEnd - nBegin + 1);//平均每个字的宽度
			 TempPlace = (g_CurMousePos.x - StringRect.left) * (nEnd - nBegin + 1) / (StringRect.right - StringRect.left);//鼠标到矩形框左边的长度（占几个字符）
			 GetStringRect(hDC, szBuff, TempPlace,     x, y, &CaretPrevRect, lpDx);//鼠标到矩形左边框
			 GetStringRect(hDC, szBuff, TempPlace + 1, x, y, &CaretNextRect, lpDx);
			 //判断鼠标聚焦的是上个矩形框还是下个矩形框，具体规则是在CalcCaretInThisPlace中定义的-3到1的范围
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
			 if (g_CurMousePos.x > CaretNextRect.right)
			 {
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

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

DWORD CalculateCaretPlaceW(HDC   hDC, 
						   LPCWSTR lpWideCharStr, 
						   UINT cbWideChars, 
						   int   x, 
						   int   y, 
						   CONST INT *lpDx,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   nCharType)
{
	RECT  StringRect, BeginRect;
	RECT  CaretPrevRect, CaretNextRect;
	double itemWidth; 
	int   TempPlace;
	int   i;

	int cbLen;
	char szBuff[256];

	wchar_t lpTemp[256];
	int nTempLen;

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;

	if ((nCharType == CHAR_TYPE_HZ) && (nBegin == nEnd))
	{
		g_nCurCaretPlace = -1;
		return 0L;
	}

	nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, nBegin, lpTemp, 256);
	GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &BeginRect, lpDx);

	nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, nEnd + 1, lpTemp, 256);
	GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &StringRect, lpDx);

	StringRect.left = BeginRect.right;
    if (StringRect.left == StringRect.right)
    {
		g_nCurCaretPlace = -1;
		return 0L;
    }
	
	switch (nCharType)
	{
		case CHAR_TYPE_HZ:
			 itemWidth = ((double)StringRect.right - (double)StringRect.left + 1) / ((double)nEnd - (double)nBegin + 1);
			 for (i = 0; i <= (nEnd - nBegin + 1); i++)
			 {
			 	if (CalcCaretInThisPlace(g_CurMousePos.x, (double)((double)StringRect.left + (double)(itemWidth * i))))
			 	{
				 	g_nCurCaretPlace = i;
				 	i = nEnd - nBegin + 2;
			 	}
			 }
             break;
		case CHAR_TYPE_ASCII:
			 itemWidth = (StringRect.right - StringRect.left + 1) / (nEnd - nBegin + 1);
			 TempPlace = (g_CurMousePos.x - StringRect.left) * (nEnd - nBegin + 1) / (StringRect.right - StringRect.left);

			 nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, TempPlace, lpTemp, 256);
			 GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretPrevRect, lpDx);
			 nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, TempPlace + 1, lpTemp, 256);
			 GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretNextRect, lpDx);
			 
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
			 if (g_CurMousePos.x > CaretNextRect.right)
			 {
				 GetEngLishCaretPlaceW(hDC, 
									  lpWideCharStr,
									  cbWideChars,
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
				 GetEngLishCaretPlaceW(hDC, 
									  lpWideCharStr,
									  cbWideChars,
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

DWORD GetEngLishCaretPlace(HDC   hDC, 
						   LPSTR szBuff,
						   int   x,
						   int   y,
						   CONST INT *lpDx,
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

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

DWORD GetEngLishCaretPlaceW(HDC   hDC, 
							LPCWSTR lpWideCharStr, 
							UINT cbWideChars, 
							int   x,
							int   y,
							CONST INT *lpDx,
							int   nBegin,    // = nPrevWord + 1
							int   nEnd,
							int   TempPlace,
							int   turnto)
{
	int i;
	RECT  CaretPrevRect, CaretNextRect;

	int cbLen;
	char szBuff[256];

	wchar_t lpTemp[256];
	int nTempLen;

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;
	
	if (turnto == RIGHT)
	{
		i = TempPlace;

		nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i, lpTemp, 256);
		GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretPrevRect, lpDx);
		
		for (i = TempPlace; i <= nEnd; i++)
		{
			 nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i + 1, lpTemp, 256);
			 GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretNextRect, lpDx);
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

		nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i + 1, lpTemp, 256);
		GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretNextRect, lpDx);

		for (i = TempPlace; i >= nBegin; i--)
		{
 			 nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i, lpTemp, 256);
			 GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretPrevRect, lpDx);
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
//判断鼠标是否聚焦字
BOOL CalcCaretInThisPlace(int CaretX, double nPlace)
{
/*	if ((double)CaretX == nPlace)
	{
		return TRUE;
	}
*/	
	if (((double)CaretX >= nPlace - 3)&&((double)CaretX <= nPlace + 1))//-3到1的范围
	{
		return TRUE;
	}
	
	return FALSE;
}

int GetHZBeginPlace(LPSTR lpszHzBuff, int nBegin, int nEnd, LPRECT lpStringRect)
{
	int itemWidth; 
	int nReturn;

	itemWidth = (lpStringRect->right - lpStringRect->left + 1) / (nEnd - nBegin + 1);
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

void AddToTotalWord(LPSTR szBuff, 
					int   cbLen, 
					int   nBegin,
					int   nEnd,
					int   nCharType,
					RECT  StringRect,
					BOOL  bInCurWord)
{
	int nPos = 0;
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "........... AddToTotalWord : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/
	if ((nCharType == CHAR_TYPE_OTHER) && (!g_bMouseInTotalWord))
	{
		g_szTotalWord[0] = 0x00;
		g_CharType = nCharType;
		g_bMouseInTotalWord = FALSE;
		return;
	}

	if (((BYTE)(szBuff[nBegin]) >= 0xa1 && (BYTE)(szBuff[nBegin]) <= 0xa9)
		&& (!g_bMouseInTotalWord))
	{
		g_szTotalWord[0] = 0x00;
		g_CharType = CHAR_TYPE_HZ;
		g_bMouseInTotalWord = FALSE;
		return;
	}
	
	if ((g_szTotalWord[0] == 0x00)&&(nCharType != CHAR_TYPE_OTHER))
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
			g_nPhraseCharType = nCharType ;
			g_nWordsInPhrase++;

			g_rcFirstWordRect.left   = StringRect.left;
			g_rcFirstWordRect.right  = StringRect.right;
			g_rcFirstWordRect.top    = StringRect.top;
			g_rcFirstWordRect.bottom = StringRect.bottom;
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
				g_nPhraseCharType = nCharType ;
				g_nWordsInPhrase++;

				g_rcFirstWordRect.left   = StringRect.left;
				g_rcFirstWordRect.right  = StringRect.right;
				g_rcFirstWordRect.top    = StringRect.top;
				g_rcFirstWordRect.bottom = StringRect.bottom;
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

	if ((g_CharType != nCharType))
	{
		if ( ((nCharType == CHAR_TYPE_OTHER) && (szBuff[nBegin] == ' '))
			 || ((nCharType != CHAR_TYPE_OTHER)
				 && (g_nPhraseCharType == nCharType)) )
		{
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
	}
	
	if (  ((UINT)(StringRect.left - g_TotalWordRect.right) <= (UINT)SEPERATOR)
		&&(abs((int)(StringRect.bottom - g_TotalWordRect.bottom)) <= SEPERATOR))
	{
		if ((BYTE)(szBuff[nBegin]) >= 0xa1 
			&& (BYTE)(szBuff[nBegin]) <= 0xa9)
			
		{
			return;
		}

		if (g_bMouseInTotalWord && g_bPhraseNeeded
			&& (g_nWordsInPhrase > MIN_WORDS_IN_PHRASE)
			&& (g_nWordsInPhrase < MAX_WORDS_IN_PHRASE)
			&& (szBuff[nBegin] == ' '))
		{
			nPos = nBegin ;
			while (szBuff[nPos] == ' ' && nPos <= nEnd)
			{
				nPos++;
			}      
			if (nPos <= nEnd || nPos - nBegin > 1)			
			{
				g_nWordsInPhrase = MAX_WORDS_IN_PHRASE + 1 ;
			}
			else
			{
				g_nWordsInPhrase++;
			}
			
		}

		if (g_nWordsInPhrase >= MAX_WORDS_IN_PHRASE)
		{
			return ;
		}

		if ((g_nCurCaretPlace != -1)&&(g_nCurCaretPlaceInTotalWord == -1))
		{
			g_nCurCaretPlaceInTotalWord = strlen(g_szTotalWord) + g_nCurCaretPlace;
		}
		CopyWord(g_szTotalWord + strlen(g_szTotalWord), szBuff, nBegin, nEnd);
		g_TotalWordRect.right  = StringRect.right;

		if (!strchr(g_szTotalWord, ' ') && (*(szBuff + nBegin) != ' '))
		{
			g_rcFirstWordRect.right  = StringRect.right;
		}

		if (bInCurWord)
		{
			g_bMouseInTotalWord = TRUE;			
			g_nPhraseCharType = nCharType ;
			g_nWordsInPhrase++;
			
			g_rcFirstWordRect.left   = StringRect.left;
			g_rcFirstWordRect.right  = StringRect.right;
			g_rcFirstWordRect.top    = StringRect.top;
			g_rcFirstWordRect.bottom = StringRect.bottom;
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
				g_bMouseInTotalWord = TRUE;
				g_nPhraseCharType = nCharType;
				g_nWordsInPhrase++;

				g_rcFirstWordRect.left   = StringRect.left;
				g_rcFirstWordRect.right  = StringRect.right;
				g_rcFirstWordRect.top    = StringRect.top;
				g_rcFirstWordRect.bottom = StringRect.bottom;
		}
	}
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "........... AddToTotalWord : %s %d %d %d %d\n", 
			g_szTotalWord, g_CurWordRect.left, g_CurWordRect.top, g_CurWordRect.right, g_CurWordRect.bottom);
		OutputDebugString(cBuffer);
	}
*/
}

void AddToTextOutBuffer(HDC hMemDC, LPSTR szBuff, int cbLen, int x, int y, CONST INT *lpDx)
{
	int  nPrevWord, nCurrentWord, CharType;
	RECT PrevWordRect, NextWordRect;
	int  nLen, i;
/*	
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "4........... AddToTextOutBuffer : (%s) %d\n", szBuff, cbLen);
		OutputDebugString(cBuffer);
	}
*/
	if (cbLen >= (int)(BUFFERLENGTH - strlen(szMemDCWordBuff) - 2))
	{
		return;
	}

	nLen = strlen(szMemDCWordBuff);
	strncpy(szMemDCWordBuff + nLen, szBuff, cbLen);
	szMemDCWordBuff[nLen + cbLen    ] = ' ';
	szMemDCWordBuff[nLen + cbLen + 1] = 0x00;

	nPrevWord = nCurrentWord = -1;

	GetStringRect(hMemDC, szBuff, nPrevWord + 1, x, y, &PrevWordRect, lpDx);//zhhn

	while (nCurrentWord < cbLen)
	{
		if (nWordNum >= MEMDC_MAXNUM)
			break;

		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);

		//GetStringRect(hMemDC, szBuff, nPrevWord + 1, x, y, &PrevWordRect, lpDx);//modified by zhhn
		GetStringRect(hMemDC, szBuff, nCurrentWord + 1 , x, y, &NextWordRect, lpDx);
		
		WordBuffer[nWordNum].nBegin = nLen + nPrevWord + 1;
		WordBuffer[nWordNum].nEnd   = nLen + nCurrentWord;
		WordBuffer[nWordNum].hMemDC = hMemDC;
		WordBuffer[nWordNum].CharType = CharType;
		WordBuffer[nWordNum].wordRect.left   = PrevWordRect.right;
		WordBuffer[nWordNum].wordRect.right  = NextWordRect.right;
		WordBuffer[nWordNum].wordRect.top    = NextWordRect.top;
		WordBuffer[nWordNum].wordRect.bottom = NextWordRect.bottom;

		CopyRect(&PrevWordRect, &NextWordRect);//zhhn

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

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

void AddToTextOutBufferW(HDC hMemDC, LPCWSTR lpWideCharStr, UINT cbWideChars, int x, int y, CONST INT *lpDx)
{
	int  nPrevWord, nCurrentWord, CharType;
	RECT PrevWordRect, NextWordRect;
	int  nLen, i;
	int cbLen;
	char szBuff[256];

	wchar_t lpTemp[256];
	int nTempLen;

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;

/*	
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "4........... AddToTextOutBufferW : (%s) %d\n", szBuff, cbLen);
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("AddToTextOutBufferW : szBuff(%s), cbLen(%d)\n", szBuff, cbLen);

	if (cbLen >= (int)(BUFFERLENGTH - strlen(szMemDCWordBuff) - 2))
	{
		return;
	}

	nLen = strlen(szMemDCWordBuff);
	strncpy(szMemDCWordBuff + nLen, szBuff, cbLen);
	szMemDCWordBuff[nLen + cbLen    ] = ' ';
	szMemDCWordBuff[nLen + cbLen + 1] = 0x00;

	nPrevWord = nCurrentWord = -1;

	GetStringRectW(hMemDC, lpWideCharStr, 0, x, y, &PrevWordRect, lpDx);//zhhn

	while (nCurrentWord < cbLen)
	{
		if (nWordNum >= MEMDC_MAXNUM)
			break;

		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);

		nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, nCurrentWord + 1, lpTemp, 256);
		GetStringRectW(hMemDC, lpWideCharStr, nTempLen , x, y, &NextWordRect, lpDx);
		
		WordBuffer[nWordNum].nBegin = nLen + nPrevWord + 1;
		WordBuffer[nWordNum].nEnd   = nLen + nCurrentWord;
		WordBuffer[nWordNum].hMemDC = hMemDC;
		WordBuffer[nWordNum].CharType = CharType;
		WordBuffer[nWordNum].wordRect.left   = PrevWordRect.right;
		WordBuffer[nWordNum].wordRect.right  = NextWordRect.right;
		WordBuffer[nWordNum].wordRect.top    = NextWordRect.top;
		WordBuffer[nWordNum].wordRect.bottom = NextWordRect.bottom;

		CopyRect(&PrevWordRect, &NextWordRect);//zhhn

		nWordNum++;

		if (nCurrentWord >= cbLen - 1)
			break;
	}
	
	GetStringLeftRight(hMemDC, szBuff, 0, x, &PrevWordRect, lpDx);
	for (i = 0; i < cbLen; i++)
	{
		nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i + 1, lpTemp, 256);
		GetStringLeftRightW(hMemDC, lpWideCharStr, nTempLen, x, &NextWordRect, lpDx);

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
		lpStringRect->right = pnMemDCCharRight[nNum];
	}
	
	WndPos.x = g_dwDCOrg.x;
	WndPos.y = g_dwDCOrg.y;
	
	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

void CheckMemDCWordBuffer(HDC hdcdest, HDC hdcSrc)
{
	int i;
	DWORD dwReturn;
	
	for (i = 0; i < nWordNum; i++)
	{
		if (WordBuffer[i].hMemDC == hdcSrc)
		{
			dwReturn = CheckMouseInMemDCWord(i);
		}
		else
		{
			if (CheckDCWndClassName(hdcdest))
			{
				dwReturn = CheckMouseInMemDCWord(i);
			}
		}

		//added by zhhn on 2000.2.2

		if (dwReturn == HAS_CURMOUSEWORD || g_bMouseInTotalWord)
		{
			PostMessage(g_hNotifyWnd, BL_HASSTRING, 0, 0l);
			//break;	//Modified by ZHHN on 2000.4 in order to get phrase
		}

		//added end
	}                               
}

__inline BOOL CheckDCWndClassName(HDC hDC)
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

DWORD CheckMouseInMemDCWord(int nWordCode)
{
	RECT  StringRect;

	GetMemWordStringRect(nWordCode, MEMDC_TOTALWORD, &StringRect);

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

/*
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "We got the word here\n");
			OutputDebugString(cBuffer);
		}
*/

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

DWORD CalculateCaretPlaceInMemDCWord(int nWordCode)
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
		g_nCurCaretPlace = 0;
		return 0l;
	}

	if (CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right))
	{
		g_nCurCaretPlace = WordBuffer[nWordCode].nEnd - WordBuffer[nWordCode].nBegin + 1;
		return 0l;
	}
	
	for (i = WordBuffer[nWordCode].nBegin; i <= WordBuffer[nWordCode].nEnd; i++)
	{
		GetMemWordStringRect(nWordCode, i - WordBuffer[nWordCode].nBegin, &StringRect);
		if (CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right))
		{
			g_nCurCaretPlace = i - WordBuffer[nWordCode].nBegin + 1;
			return 0l;
		}
	}
	
	return 0L;
}