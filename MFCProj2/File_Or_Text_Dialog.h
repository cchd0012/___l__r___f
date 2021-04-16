#pragma once
#include "afxwin.h"


// File_Or_Text_Dialog 대화 상자입니다.

class File_Or_Text_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(File_Or_Text_Dialog)

public:
	File_Or_Text_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~File_Or_Text_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_F_T_SELECT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CButton m_FN_Radio;
	CButton m_TE_Radio;

	unsigned int Type;
	virtual void OnOK();
};
