
#include <windows.h>
#include "NHTW32.H"
#include "import.h"

#include "getver.h"
#include "HookApi.h"

//#define MUTEXNAME "NOBLE HAND"
#define DLL32NAME "NHW32.DLL"
#define DLL16NAME "NHW16.DLL"

extern APIHOOKSTRUCT g_ExtTextOutWHook;

extern HANDLE hMutex;

//*********************************************************************
//Added by XGL, Jan 7th, 1999
//for get word in Acrobat Reader

HANDLE g_heventGetWord = NULL ;	//event handle

HANDLE g_hMappedFile = NULL ;	//handle of file-mapping object

void *g_pMemFile = NULL ;		//pointer to memfile

//flag indicates if the global variables
//for getting word in Acrobat Reader have
//been initialized.	
BOOL g_bAcroReaderInit = FALSE ;

//*********************************************************************
//function that initializes the global variables
//for getting word in Acrobat Reader.
//*********************************************************************
BOOL InitForAcrobatR()
{
	BOOL bMappingExisted = FALSE ;

	//just initialize once
	if (g_bAcroReaderInit)
	{
		return TRUE ;
	}

	//open existed mapping-file
	g_hMappedFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,
									FALSE, MEM_FILE_NAME) ;
	if (NULL == g_hMappedFile)
	{
		//create a new mapping-file
		g_hMappedFile = CreateFileMapping((HANDLE)0xffffffff,
			NULL, PAGE_READWRITE, 0, MEM_FILE_SIZE, MEM_FILE_NAME) ;
		if (NULL == g_hMappedFile)
		{
			return FALSE ;
		}
	}
	else
	{
		bMappingExisted = TRUE ;
	}

	//map the file into our address
	g_pMemFile = MapViewOfFile(g_hMappedFile, FILE_MAP_WRITE,
									0, 0, 0) ;
	if (NULL == g_pMemFile)
	{
		CloseHandle(g_hMappedFile) ;
		g_hMappedFile = NULL ;
		return FALSE ;
	}

	if (!bMappingExisted)
	{
		//set plug-in unexisted flag
		*((BYTE*)(g_pMemFile) + PLUGIN_FLAG_POS) = NOT_EXISTED ;
	}

	//set mainproc existed flag 
	*((BYTE*)(g_pMemFile) + MAINPROC_FLAG_POS) = EXISTED ;

	//open getword event
	g_heventGetWord = OpenEvent(EVENT_ALL_ACCESS,
								FALSE, GET_WORD_EVENT) ;
	if (NULL == g_heventGetWord)
	{
		//if it does not exist, create it
		g_heventGetWord = CreateEvent(NULL, TRUE, TRUE, GET_WORD_EVENT) ;
		if (NULL == g_heventGetWord)
		{
			if (NULL != g_hMappedFile)
			{
				CloseHandle(g_hMappedFile) ;
				g_hMappedFile = NULL ;
			}
			if (NULL != g_pMemFile)
			{
				UnmapViewOfFile(g_pMemFile) ;
				g_pMemFile = NULL ;
			}

			return FALSE;
		}
	}

	g_bAcroReaderInit = TRUE ;
	return TRUE ;
}

//*********************************************************************
//function that uninitializes the global variables
//for getting word in Acrobat Reader.
//*********************************************************************
void UninitForAcrobatR()
{
	if (g_bAcroReaderInit)
	{
		//set exit flag
		*((BYTE*)(g_pMemFile) + MAINPROC_FLAG_POS) = NOT_EXISTED ;

		//close handles
		if (NULL != g_heventGetWord)
		{
			CloseHandle(g_heventGetWord) ;
			g_heventGetWord = NULL ;
		}
		if (NULL != g_hMappedFile)
		{
			CloseHandle(g_hMappedFile) ;
			g_hMappedFile = NULL ;
		}
		if (NULL != g_pMemFile)
		{
			UnmapViewOfFile(g_pMemFile) ;
			g_pMemFile = NULL ;
		}
	}
	g_bAcroReaderInit = FALSE ;
}
//end of addition, XGL, Jan 7th, 1999
//*********************************************************************


BOOL _stdcall thk_ThunkConnect32(LPSTR      pszDll16,
                                 LPSTR      pszDll32,
                                 HINSTANCE  hInst,
                                 DWORD      dwReason);

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
	g_ExtTextOutWHook.hInst = hDLLInst;

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			if (!(thk_ThunkConnect32(DLL16NAME,
									 DLL32NAME,
									 hDLLInst,
									 fdwReason)))
			{
				return FALSE;
			}

//			DbgPrintf("NhW32.dll:          DLL_PROCESS_ATTACH");
			if (IsDesiredFileVersion("GDI.EXE", CHINESE_TAIWAN))
			{
				BL_SetVer16(CHINESE_TAIWAN);
			}

			if (IsDesiredFileVersion("GDI.EXE", CHINESE_PRC))
			{
				BL_SetVer16(CHINESE_PRC);
			}
			hMutex = CreateMutex(NULL, FALSE, MUTEXNAME);
			if (hMutex == NULL)
			{
				return FALSE;
			}
			//added by XGL, Jan 7th, 1999
			if (!InitForAcrobatR())
			{
				return FALSE ;
			}
		}
		//            break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			RestoreWin32Api(&g_ExtTextOutWHook, HOOK_ONLY_READ);

			if (!(thk_ThunkConnect32(DLL16NAME,
									 DLL32NAME,
									 hDLLInst,
									 fdwReason)))
			{
				return FALSE;
			}

			CloseHandle(hMutex);
			//added by XGL, Jan 7th, 1999
			UninitForAcrobatR() ;
			break;

		case DLL_THREAD_DETACH:
			if (!(thk_ThunkConnect32(DLL16NAME,
									 DLL32NAME,
									 hDLLInst,
									 fdwReason)))
			{
				return FALSE;
			}

			break;
	}
	return TRUE;
}

