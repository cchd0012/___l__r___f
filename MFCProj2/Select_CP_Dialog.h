#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Select_CP_Dialog 대화 상자입니다.

class Select_CP_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Select_CP_Dialog)

public:
	Select_CP_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Select_CP_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CP_SELECT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CButton m_CP_Radio1;
	CButton m_CP_Radio2;
	unsigned int CP_Code;

	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
};
