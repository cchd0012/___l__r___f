#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Change_FDName_Dialog 대화 상자입니다.

class Change_FDName_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Change_FDName_Dialog)

public:
	Change_FDName_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Change_FDName_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CHANGE_FD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_Input_Name;
	CEdit m_Output_Name;

	bool ShouldChange;

	CString Input_WStr;
	CString Output_WStr;
	unsigned int Meta_type;
	unsigned int code;
	//*밖에서 받거나 내보낼 때 쓸 변수들

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};
