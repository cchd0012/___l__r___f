#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Pack_Dialog 대화 상자입니다.

class Pack_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Pack_Dialog)

public:
	Pack_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Pack_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_PACK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_Wolf_Package_Name;

	afx_msg void OnBnClickedPack();

	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	CButton m_CP_Radio1;
	CButton m_CP_Radio2;

	CButton m_Type_Radio1;
	CButton m_Type_Radio2;
	CButton m_Type_Radio3;
	CButton m_Type_Radio4;
	CButton m_Type_Radio5;
	
	CStatic m_Pack_Print_String;
	CWinThread *Pack_Thread;
	static UINT Pack_Func (LPVOID lParam);
	CProgressCtrl m_Pack_Progress;
};
