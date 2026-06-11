/******************************************************************************
Modify for vs2008 (06/05/2009)
/******************************************************************************/
#if !defined(AFX_TFCSERVERDLG_H__BC8F3068_A74F_11D0_9B9E_444553540000__INCLUDED_)
#define AFX_TFCSERVERDLG_H__BC8F3068_A74F_11D0_9B9E_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/******************************************************************************/
// CTFCServerDlg dialog
class CTFCServerDlg : public CDialog
/******************************************************************************/
{
	enum { IDD = IDD_TFCMAIN_DIALOG };
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	CTFCServerDlg(CWnd* pParent = NULL);	// standard constructor	

    void TerminateServer( bool boExit = true );

protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
public:
    afx_msg void OnClose();
protected:
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_TFCSERVERDLG_H__BC8F3068_A74F_11D0_9B9E_444553540000__INCLUDED_)
