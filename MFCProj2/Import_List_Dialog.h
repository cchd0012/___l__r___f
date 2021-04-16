#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Import_List_Dialog 대화 상자입니다.

class Import_List_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Import_List_Dialog)

public:
	Import_List_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Import_List_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_IMPORT_LIST_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClicked_ImportFromDir();
	afx_msg void OnBnClicked_ImportFromFile();
};
