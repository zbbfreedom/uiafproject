#include <windows.h>
#include <limits.h>

#include "GetVer.h"

BOOL GetProgramVersion(LPTSTR szFileName,
                       int nVersionMin[3],
                       int nVersionMax[3],
                       DWORD dwLangId
                       )
{
        DWORD dwZero, dwVerInfoSize;
        LPVOID lpData;
        LPVOID lpBuffer;
        UINT uBytes;
        VS_FIXEDFILEINFO *pVsFixedFileInfo;
        INT nVersionNums[3], i;
        WORD* pdwLangIds;
        BOOL boolIsLangSupported = FALSE;

        if(szFileName == NULL)
                return FALSE;

        dwVerInfoSize = GetFileVersionInfoSize(szFileName, &dwZero);
        if(!dwVerInfoSize)
                return FALSE;
        
        lpData = HeapAlloc(GetProcessHeap(), 0, dwVerInfoSize);
        if(!lpData)
                return FALSE;
        __try {
                if(!GetFileVersionInfo(szFileName, 0, dwVerInfoSize, lpData))
                        return FALSE;

                if(!VerQueryValue(lpData, "\\", &lpBuffer, &uBytes))
                        return FALSE;

                if( uBytes == 0 )
                        return FALSE;
                
                pVsFixedFileInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                nVersionNums[0] = HIWORD(pVsFixedFileInfo-> dwFileVersionMS);
                nVersionNums[1] = LOWORD(pVsFixedFileInfo->dwFileVersionMS);
                nVersionNums[2] = pVsFixedFileInfo->dwFileVersionLS;

                if(dwLangId == 0)
                        __leave;

                if(!VerQueryValue(lpData, "\\VarFileInfo\\Translation", &lpBuffer, &uBytes))
                        return FALSE;

                if(uBytes == 0)
                        return FALSE;
                
                pdwLangIds = (WORD *)lpBuffer;

                for(i = 0; i < (INT)(uBytes/sizeof(WORD)); i++)
                {
                        if(pdwLangIds[i] == dwLangId)
                        {
                                boolIsLangSupported = TRUE;
                                break;
                        }
                }

                if( !boolIsLangSupported )
                        return FALSE;

        } __finally {
                if(lpData)
                        HeapFree(GetProcessHeap(), 0, lpData);
        }

        return TRUE;
}

BOOL IsDesiredFileVersion(LPTSTR szFileExeName, DWORD dwLangId)
{
        int vermin[3] = {IE_VERSION_FIRST, IE_VERSION_SECOND, 0},
                        vermax[3] = {IE_VERSION_FIRST, IE_VERSION_SECOND, INT_MAX};
        return GetProgramVersion(szFileExeName, vermin, vermax, dwLangId);
}

