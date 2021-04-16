#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Extract_Dialog 대화 상자입니다.

class Extract_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Extract_Dialog)

public:
	Extract_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Extract_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_EXTRACT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelectButton();
	CString Wolf_Full_Path;
	CString Wolf_File_Name;
	CString Wolf_File_Title;

	afx_msg void OnBnClickedExtract();

	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	CButton m_CP_Radio1;
	CButton m_CP_Radio2;
	
	CStatic m_Extract_Print_String;
	CWinThread *Extract_Thread;
	static UINT Extract_Func (LPVOID lParam);
	CProgressCtrl m_Extract_Progress;
};