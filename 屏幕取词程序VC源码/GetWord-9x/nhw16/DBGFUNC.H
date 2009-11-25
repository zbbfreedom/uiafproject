#ifndef __DBG_FUNC_FILE__
#define __DBG_FUNC_FILE__

#define _DEBUG_FILE_NAME_ "C:\\DEBUG.OUT"
#define _CHECK_END_FILE_ "C:\\END.OUT" 
#define _DBGFILE_OVERWRITE_

#include "stdio.h"

BOOL IsErrPara(FILE* fp, LPSTR szBuff);
void DbgPrintf(LPSTR fmt, ...);
void DbgLPCSTR(LPSTR szPreWrite, LPSTR szBuff, int cbLen, BOOL bReturn);

#endif // __DBG_FUNC_FILE__