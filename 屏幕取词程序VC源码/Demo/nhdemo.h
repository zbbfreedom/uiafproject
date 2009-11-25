// nhdemo.h : main header file for the NHDEMO application
//

#if !defined(AFX_NHDEMO_H__F393DD66_3D66_4B26_AB2B_7105E4137ABB__INCLUDED_)
#define AFX_NHDEMO_H__F393DD66_3D66_4B26_AB2B_7105E4137ABB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CNhdemoApp:
// See nhdemo.cpp for the implementation of this class
//

class CNhdemoApp : public CWinApp
{
public:
	CNhdemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNhdemoApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNhdemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NHDEMO_H__F393DD66_3D66_4B26_AB2B_7105E4137ABB__INCLUDED_)
