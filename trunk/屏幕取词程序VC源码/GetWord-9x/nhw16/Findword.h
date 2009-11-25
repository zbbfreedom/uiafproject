#ifndef __YAN_H__
#define __YAN_H__

#define HAS_CURMOUSEWORD  1
#define NO_CURMOUSEWORD   0

#define CHAR_TYPE_ASCII 0           // the current character is a...z or A...Z
#define CHAR_TYPE_HZ    1           // the currnet character is chinese.
#define CHAR_TYPE_OTHER 2           // other character.
#define CHAR_LINK       '-'
#define CHAR_WILDCHAR1 '*'
#define CHAR_WILDCHAR2 '?'

//#define SEPERATOR       2
#define SEPERATOR       4

#define MAX_CHAR_HEIGHT 40

int  GetCharType(char ch);
int  FindAWord(LPCSTR lpString, int nFromPlace, int nLength);
int  FindDWord(LPCSTR lpString, int nFromPlace, int nLength);
int  FindTWWord(LPCSTR lpString, int nFromPlace, int nLength);
BOOL IsASCIIWord(LPCSTR lpString, int nFromPlace, int nLength, int nCurCharNum);
int  FindHZWord(LPCSTR lpString, int nFromPlace, int nLength);
int  FindNextWordBegin(LPCSTR lpString, int nFromPlace, int nLength);
int  GetCurWordEnd(LPCSTR lpString, int nFromPlace, int nLength, int nCharType);
void CopyWord(LPSTR lpWord, LPCSTR lpString, int nBegin, int nEnd);

void GetStringTopBottom(HDC hDC, int y, RECT* lpStringRect);
void GetStringLeftRight(HDC hDC, LPSTR szBuff, int cbLen, int x, RECT* lpStringRect, int FAR* lpDx);
void GetStringRect(HDC hDC, LPSTR szBuff, int cbLen, int x, int y, RECT* lpStringRect, int FAR* lpDx);

DWORD GetCurMousePosWord(HDC   hDC, 
						 LPSTR szBuff, 
						 int   cbLen, 
						 int   x, 
						 int   y, 
						 int   FAR* lpDx);

DWORD CheckMouseInCurWord(HDC   hDC, 
						 LPSTR szBuff, 
						 int   cbLen, 
						 int   x, 
						 int   y, 
						 int   FAR* lpDx,
						 int*  lpLeft,
						 int   nBegin,    // = nPrevWord + 1
						 int   nEnd,
						 int   nCharType);
DWORD CalculateCaretPlace(HDC   hDC, 
						   LPSTR szBuff, 
						   int   cbLen, 
						   int   x, 
						   int   y, 
						   int   FAR* lpDx,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   nCharType);
DWORD GetEngLishCaretPlace(HDC   hDC, 
						   LPSTR szBuff,
							int   x,
							int   y,
							int   FAR* lpDx,
							int   nBegin,    // = nPrevWord + 1
							int   nEnd,
							int   TempPlace,
							int   turnto);
int GetHZBeginPlace(LPSTR lpszHzBuff, int nBegin, int nEnd, LPRECT StringRect);
void AddToTotalWord(LPSTR szBuff, 
					int   cbLen, 
					int   nBegin,
					int   nEnd,
					int   nCharType,
					RECT  StringRect,
					BOOL  bInCurWord);

//BOOL CalcCaretInThisPlace(int CaretX, int nPlace);
BOOL CalcCaretInThisPlace(int CaretX, double nPlace);

// deal with memdc string.
#define MEMDC_TOTALWORD -1

void AddToTextOutBuffer(HDC hMemDC, LPSTR szBuff, int cbLen, int x, int y, int FAR* lpDx);
void GetMemWordStringRect(int nWordCode, int nOffset, LPRECT lpStringRect);
void CheckMemDCWordBuffer(HDC hdcDest, HDC hdcSrc);
DWORD CheckMouseInMemDCWord(int nWordCode);
DWORD CalculateCaretPlaceInMemDCWord(int nWordCode);

#ifdef _DICTING_
BOOL CheckDCWndClassName(HDC hDC);
#endif

#endif
