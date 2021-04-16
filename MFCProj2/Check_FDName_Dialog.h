#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Check_FDName_Dialog 대화 상자입니다.

class Check_FDName_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Check_FDName_Dialog)

public:
	Check_FDName_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Check_FDName_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CHECK_FD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	unsigned int code;

	CStatic m_Check_FDName_Print;
	CWinThread *Save_Text_Thread;
	static UINT Check_FDName_Func (LPVOID lParam);
	CProgressCtrl m_Check_FDName_Progress;
};
