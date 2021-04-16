#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Edit_Text_Dialog 대화 상자입니다.

class Edit_Text_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Edit_Text_Dialog)

public:
	Edit_Text_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Edit_Text_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_EDIT_TEXT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:

	unsigned int m_Input_CP, m_Output_CP;
	CString m_Input_tmp, m_Output_tmp;
	//*초기화할 때와 값을 받아올 때 쓴다

	CComboBox m_Input_ComboBox;
	CComboBox m_Output_ComboBox;
	//*언어코드 지정하는 콤보박스

	CEdit m_Input_Text;
	CEdit m_Output_Text;
	//*각각 입력받은 텍스트, 적용할 텍스트(적용할 텍스트는 비면 안 된다)
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
