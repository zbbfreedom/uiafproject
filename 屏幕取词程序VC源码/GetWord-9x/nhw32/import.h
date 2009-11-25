
#ifndef __IMPORT_H__
#define __IMPORT_H__

DWORD PASCAL BL_SetVer16(int nLangVer);
DWORD PASCAL BL_SetGetWordStyle(int nGetWordStyle);
DWORD PASCAL BL_SetPara16(short hNotifyWnd, short MouseX, short MouseY);
DWORD PASCAL BL_GetBuffer16(LPSTR lpszBuffer, short nBufferSize, LPWORDRECT lpWr);
DWORD PASCAL BL_HookWinApi16();
DWORD PASCAL BL_UnHookWinApi16();
void  PASCAL Nh16_AddToTotalWord(LPSTR  szBuff,
								 int    cbLen,
								 int    nBegin,
								 int    nEnd,
								 int    nCharType,
								 LPRECT lpStringRect,
								 BOOL   bInCurWord,
								 int    nCurCaret);
int  PASCAL Nh16_GetTempLen();
BOOL PASCAL Nh16_AddToTempBuff(LPSTR  szBuff,
			                   int    cbLen);
BOOL PASCAL Nh16_AddToWordStruct(int nBegin,
								 int nEnd,
								 HDC hMemDC, 
								 int CharType, 
								 LPRECT lpStringRect);
BOOL PASCAL Nh16_AddToCharStruct(int nBeignPlace, int nLeft, int nRight);

#endif  // __IMPORT_H__
