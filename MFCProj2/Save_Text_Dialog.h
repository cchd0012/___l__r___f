#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Save_Text_Dialog 대화 상자입니다.

class Save_Text_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Save_Text_Dialog)

public:
	Save_Text_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Save_Text_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SAVE_TEXT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	CString m_FP_tmp;
	
	CWinThread *Save_Text_Thread;
	static UINT Save_Text_Func (LPVOID lParam);
	CProgressCtrl m_Save_Text_Progress;
};
