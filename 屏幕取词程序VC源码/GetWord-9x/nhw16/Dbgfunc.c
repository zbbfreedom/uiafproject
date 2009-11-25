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

BOOL bDebugBegin = TRUE;

// This function is used to check whether file handle and the buffer is right.
// right then return FALSE, Otherwise return FALSE;
BOOL IsErrPara(FILE* fp, LPSTR szBuff)
{
	// Check file handle.
	if (fp == NULL)
	{
		MessageBeep(MB_OK);
		return TRUE;
	}
	
	
	return FALSE;
}

// Because thunk program can not debug.
// So this function use to write debug message to file.
// And use for debug version.
void DbgPrintf(LPSTR fmt, ...)
{
	va_list marker;
	char szBuff[4096];
	FILE* fp;

	va_start(marker, fmt);
	wvsprintf(szBuff, fmt, marker);
	va_end(marker);
    
#ifdef _DBGFILE_OVERWRITE_
	if (bDebugBegin)
	{
		// Delete old debug file.
		bDebugBegin = FALSE;
		fp = fopen(_DEBUG_FILE_NAME_, "w");
		fprintf(fp, "Debug Message:\n");
		fclose(fp);
	}
#endif
	
	fp = fopen(_DEBUG_FILE_NAME_, "a");
	
	if (IsErrPara(fp, szBuff))
	{
		return;
	}	
	
	fprintf(fp, "%s\n", szBuff);
	fclose(fp); 
}

void DbgLPCSTR(LPSTR szPreWrite, LPSTR szBuff, int cbLen, BOOL bReturn)
{
	char temp[4096];
	FILE*  fp;
		
	strncpy(temp, szBuff, cbLen);
	temp[cbLen] = 0x00;
	
#ifdef _DBGFILE_OVERWRITE_
	if (bDebugBegin)
	{
		// Delete old debug file.
		bDebugBegin = FALSE;
		fp = fopen(_DEBUG_FILE_NAME_, "w");
		fprintf(fp, "Debug Message:\n");
		fclose(fp);
	}
#endif

	fp = fopen(_DEBUG_FILE_NAME_, "a");

	if (IsErrPara(fp, szBuff))
	{
		return;
	}	

	fprintf(fp, "%s%s", szPreWrite, temp);
	if (bReturn)
		fprintf(fp, "\n");
	fclose(fp); 
}
