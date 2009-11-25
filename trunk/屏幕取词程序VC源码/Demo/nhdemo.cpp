// nhdemo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "nhdemo.h"
#include "nhdemoDlg.h"
#include "getwords.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNhdemoApp

BEGIN_MESSAGE_MAP(CNhdemoApp, CWinApp)
	//{{AFX_MSG_MAP(CNhdemoApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNhdemoApp construction

CNhdemoApp::CNhdemoApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNhdemoApp object

CNhdemoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CNhdemoApp initialization

BOOL CNhdemoApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	//get operate system information

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx (&osvi);

	if(osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		//it is running on Win95/98 platform

		if(FindWindow(NULL,"NHD_FlyWindow") != NULL)
		{
			//because conflict with nhstart.exe on Win95/98 environment，then exit. 
			AfxMessageBox("Find nhstart.exe is running, please close it.");
			return FALSE;
		}
	}

	CNhdemoDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

//重载了一下MFC的消息传递函数 在这里判断是否按了Ctr按钮
BOOL CNhdemoApp::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_CONTROL)
	{
		//press Ctrl key

		POINT ptMousePos = {0, 0};
		//取得当前鼠标的位置
		if (GetCursorPos(&ptMousePos))
		{
			NHD_BeginGetWord(ptMousePos);
		}

	}
	return CWinApp::PreTranslateMessage(pMsg);
}
