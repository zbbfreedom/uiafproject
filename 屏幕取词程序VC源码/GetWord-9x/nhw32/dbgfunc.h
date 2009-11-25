#ifndef __DBG_FUNC_FILE__
#define __DBG_FUNC_FILE__

#define _DEBUG_FILE_NAME_ "C:\\DEBUG.OUT"

void DbgPrintf(LPSTR fmt, ...);
void DbgLPCSTR(LPSTR szPreWrite, LPSTR szBuff, int cbLen, BOOL bReturn);
void DbgPrintfToICE(LPTSTR fmt, ...);

#endif // __DBG_FUNC_FILE__