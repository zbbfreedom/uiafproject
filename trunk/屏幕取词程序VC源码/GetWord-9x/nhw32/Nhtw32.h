
#ifndef __NHTW32_H__
#define __NHTW32_H__

#include <windows.h>
#include "..\pub3216.h"

#define MUTEXNAME "NOBLE HAND"

//*********************************************************************
//Added by XGL, Jan 7th, 1999
//for Acrobat Reader
#define ACROBATR_VIEW_CLASS_NAME "AVL_AVView"
#define MEM_FILE_SIZE 320
#define NH_WAIT_TIME  3000

//positions in mem file buffer
#define MAINPROC_FLAG_POS	0
#define PLUGIN_FLAG_POS		sizeof(BYTE)
#define MOUSE_X_POS			PLUGIN_FLAG_POS + sizeof(BYTE)
#define MOUSE_Y_POS			MOUSE_X_POS + sizeof(int)
#define WND_HANDLE_POS		MOUSE_Y_POS + sizeof(int)
#define WND_NOTIFIY_POS		WND_HANDLE_POS + sizeof(HWND)
#define RECT_POS			WND_NOTIFIY_POS + sizeof(HWND)
#define WORD_POS			RECT_POS + 4 * sizeof(LONG)

#define MEM_FILE_NAME "NHPlugInMemFile"	//name of file-mapping object
#define GET_WORD_EVENT "NHGetWordEvent"	//name of getwrod event

#define EXISTED 1
#define NOT_EXISTED 0
//end of addition, XGL, Jan 7th, 1999
//*********************************************************************

#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C"
{
#endif

DLLEXPORT DWORD WINAPI BL_SetFlag32(UINT nFlag, HWND hNotifyWnd, int MouseX, int MouseY);
DLLEXPORT DWORD WINAPI BL_GetText32(LPSTR lpszCurWord, int nBufferSize, LPRECT lpWordRect);

DLLEXPORT BOOL WINAPI NhExtTextOutW(HDC hdc,			// handle to device context 
									int X,				// x-coordinate of reference point 
									int Y,				// y-coordinate of reference point 
									UINT fuOptions,		// text-output options 
									CONST RECT *lprc,	// optional clipping and/or opaquing rectangle 
									LPCTSTR lpString,	// points to string 
									UINT cbCount,		// number of characters in string 
									CONST INT *lpDx 	// pointer to array of intercharacter spacing values  
								   );

#ifdef __cplusplus
}
#endif

DWORD BL_ExtTextOutW(HDC hDC,
					 int X,
					 int Y,
					 UINT fuOptions,
					 LPRECT lprc,
					 LPSTR lpString,
					 UINT cbCount,
					 LPINT lpDx);

#endif  // __NHTW32_H__

