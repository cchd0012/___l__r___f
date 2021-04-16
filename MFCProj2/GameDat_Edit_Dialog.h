#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// GameDat_Edit_Dialog 대화 상자입니다.

class GameDat_Edit_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(GameDat_Edit_Dialog)

public:
	GameDat_Edit_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~GameDat_Edit_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_EDIT_GAMEDAT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();
	virtual void OnOK();

	CEdit m_Game_Title;
	CEdit m_Font1;
	CEdit m_Font2;
	CEdit m_Font3;
	CEdit m_Font4;
	//*실제로 반영할 때 쓰일 값들

	CString Game_Title;
	CString Font1;
	CString Font2;
	CString Font3;
	CString Font4;
	unsigned int type, code;
	//*받아오거나 내보낼 때 쓰일 값들

	CButton m_GD_CPRadio1;
	CButton m_GD_CPRadio2;
};
