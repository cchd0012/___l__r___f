#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#define NON_DIRECTION	0x0
#define UP_DIRECTION	0x1
#define DOWN_DIRECTION	0x2
//*방향 지정 코드

// Find_Text_Dialog 대화 상자입니다.

class Find_Text_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Find_Text_Dialog)

public:
	Find_Text_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Find_Text_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_FIND_TEXT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();
	virtual void OnOK();

	CEdit m_Find_String;
	CButton m_FT_Up_Radio;
	CButton m_FT_Down_Radio;

	unsigned int Direction;
	CString Find_Str;
	//*방향과 문자열을 받기 위한 변수

};
