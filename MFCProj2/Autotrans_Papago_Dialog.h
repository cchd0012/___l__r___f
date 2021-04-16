#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Autotrans_Papago_Dialog 대화 상자입니다.

class Autotrans_Papago_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Autotrans_Papago_Dialog)

public:
	Autotrans_Papago_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Autotrans_Papago_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_AUTO_TRANS_PAPAGO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_Src_Language_List;
	CComboBox m_Dst_Language_List;
	CComboBox m_Text_File_List;

	CEdit m_Start_Idx_Str;
	unsigned int s_idx;
	CEdit m_End_Idx_Str;
	unsigned int e_idx;

	unsigned int progressing_file_idx;
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();

	afx_msg void OnBnClickedTranslate();
	
	CStatic m_Translate_Print_String;
	CWinThread *Translate_Thread;
	static UINT Translate_Func (LPVOID lParam);
	CProgressCtrl m_Translate_Progress;
	afx_msg void OnCbnSelchangeComboTransFile();
};
