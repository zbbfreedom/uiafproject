// nhdemoDlg.h : header file
//

#if !defined(AFX_NHDEMODLG_H__004702A1_F395_4CCF_BC07_BD2EDF4EC281__INCLUDED_)
#define AFX_NHDEMODLG_H__004702A1_F395_4CCF_BC07_BD2EDF4EC281__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CNhdemoDlg dialog

class CNhdemoDlg : public CDialog
{
// Construction
public:
	CNhdemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CNhdemoDlg)
	enum { IDD = IDD_NHDEMO_DIALOG };
	CString	m_szWords;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNhdemoDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNhdemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	afx_msg LRESULT OnGetWordsOK(UINT wParam, LONG lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NHDEMODLG_H__004702A1_F395_4CCF_BC07_BD2EDF4EC281__INCLUDED_)
