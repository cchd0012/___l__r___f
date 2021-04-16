#pragma once
#include "afxwin.h"


// Replace_Text_Dialog 대화 상자입니다.

class Replace_Text_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Replace_Text_Dialog)

public:
	Replace_Text_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Replace_Text_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_REPLACE_TEXT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:

	virtual void OnOK();

	CEdit m_Replace_Text_Find;
	CEdit m_Replace_Text_Change;

	CString strtofind;
	CString strtochg;
	unsigned int code;
	bool FCH;
	//*문자열, 코드값, 파일 이름 변경 여부를 받기 위한 변수

	virtual BOOL OnInitDialog();
	CButton m_CP_Radio1;
	CButton m_CP_Radio2;
	CButton m_CP_Radio3;
	CButton m_RF_check;
};
