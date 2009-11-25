/**************************************************************************
    DbgFunc.C

    Use to write debug message to a file.
    Because some code in this dll is system code, it can not be debug.
    so I write some message to file.  Use this message to debug dll.

    (C) 1996.11 Inventec (TianJin) Co., Ltd.

    Author: Gang Yan

    Comments:  1. 97.5.30 is version 1.0.

***************************************************************************/

#include "windows.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

#include "DbgFunc.h"

//#define _WIN16_DEBUG_
#define _WIN32_DEBUG_

extern BOOL bDebugBegin;

#ifdef _WIN32_DEBUG_
void WriteWin32File(char* szBuff);
#endif
// Because thunk program can not debug.
// So this function use to write debug message to file.
// And use for debug version.
void DbgPrintf(LPSTR fmt, ...)
{
    va_list marker;
    char szBuf[4096];
	int  len;
#ifdef _WIN16_DEBUG_
    FILE* fp;
#endif

    va_start(marker, fmt);
    wvsprintf(szBuf, fmt, marker);
    va_end(marker);
	len = strlen(szBuf);
	szBuf[len] = 0x0d;
	szBuf[len + 1] = 0x0a;
	szBuf[len + 2] = 0;
    
    if (bDebugBegin)
    {
    	// Delete old debug file.
    	bDebugBegin = FALSE;
#ifdef _WIN16_DEBUG_
    	fp = fopen(_DEBUG_FILE_NAME_, "w");
    	fprintf(fp, "Debug Message:\n");
    	fclose(fp);
#endif
#ifdef _WIN32_DEBUG_
		WriteWin32File("Debug Message:\n");
#endif
    }
	
#ifdef _WIN32_DEBUG_
	WriteWin32File(szBuf);
#endif
#ifdef _WIN16_DEBUG_
	fp = fopen(_DEBUG_FILE_NAME_, "a");
	fprintf(fp, "%s", szBuf);
	fclose(fp); 
#endif
}

void DbgLPCSTR(LPSTR szPreWrite, LPSTR szBuff, int cbLen, BOOL bReturn)
{
	char temp[4096];
	FILE*  fp;
		
	strncpy(temp, szBuff, cbLen);
	temp[cbLen] = 0x00;
	
	fp = fopen(_DEBUG_FILE_NAME_, "a");
	fprintf(fp, "%s%s", szPreWrite, temp);
	if (bReturn)
		fprintf(fp, "\n");
	fclose(fp); 
}

#ifdef _WIN32_DEBUG_
void WriteWin32File(char* szBuff)
{
	HANDLE hDbgFile;
	DWORD dwWritedNum;

	hDbgFile = CreateFile(_DEBUG_FILE_NAME_,	// pointer to name of the file 
						  GENERIC_WRITE,		// access (read-write) mode 
						  FILE_SHARE_WRITE,		// share mode 
						  NULL,					// pointer to security descriptor 
						  CREATE_NEW,			// how to create 
						  FILE_ATTRIBUTE_NORMAL,	// file attributes 
						  NULL 	// handle to file with attributes to copy  
						 );

	SetEndOfFile(hDbgFile);
	WriteFile(hDbgFile,			// handle to file to write to 
			  szBuff,			// pointer to data to write to file 
			  strlen(szBuff),	// number of bytes to write 
			  &dwWritedNum,		// pointer to number of bytes written 
			  NULL			 	// pointer to structure needed for overlapped I/O
			 );

	CloseHandle(hDbgFile);
}

void DbgPrintfToICE(LPTSTR fmt, ...)
{
    va_list marker;
    TCHAR szBuf[1024];

    va_start(marker, fmt);
    wvsprintf(szBuf, fmt, marker);
    va_end(marker);

    OutputDebugString(szBuf);
    OutputDebugString(TEXT("\r\n"));
}
#endif

