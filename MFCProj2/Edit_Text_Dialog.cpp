// Edit_Text_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Edit_Text_Dialog.h"
#include "afxdialogex.h"


// Edit_Text_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Edit_Text_Dialog, CDialogEx)

Edit_Text_Dialog::Edit_Text_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Edit_Text_Dialog::IDD, pParent)
{

}

Edit_Text_Dialog::~Edit_Text_Dialog()
{
}

void Edit_Text_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ET_COMBO_BFR, m_Input_ComboBox);
	DDX_Control(pDX, IDC_ET_COMBO_AFT, m_Output_ComboBox);
	DDX_Control(pDX, IDC_ET_EDIT_BFR, m_Input_Text);
	DDX_Control(pDX, IDC_ET_EDIT_AFT, m_Output_Text);
}


BEGIN_MESSAGE_MAP(Edit_Text_Dialog, CDialogEx)
END_MESSAGE_MAP()


// Edit_Text_Dialog 메시지 처리기입니다.


BOOL Edit_Text_Dialog::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN)) {

		if (GetFocus() == GetDlgItem(IDC_ET_EDIT_AFT)) { m_Output_Text.ReplaceSel (_T("\r\n")); }
		return true; 
		//*텍스트 입력창에 포커스가 맞춰져 있을 때만 개행을 넣어준다

	}
	//*enter를 누르면 닫히지 않고 그대로 편집할 수 있게 한다

	return CDialogEx::PreTranslateMessage(pMsg);
}


BOOL Edit_Text_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	m_Input_ComboBox.AddString(_T("Japanese"));
	m_Input_ComboBox.AddString(_T("Korean"));
	m_Output_ComboBox.AddString(_T("Japanese"));
	m_Output_ComboBox.AddString(_T("Korean"));
	SetDlgItemText (IDC_ET_EDIT_BFR, m_Input_tmp);
	SetDlgItemText (IDC_ET_EDIT_AFT, m_Output_tmp);
	//*텍스트 추가 및 설정

	m_Input_ComboBox.SetCurSel (m_Input_CP);
	m_Output_ComboBox.SetCurSel (m_Output_CP);
	GetDlgItem(IDC_ET_COMBO_BFR)->EnableWindow(FALSE);
	//*m_Input_ComboBox는 편집하라고 만든 게 아니니 지정만 하고 Disable시킨다

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void Edit_Text_Dialog::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	m_Output_CP = m_Output_ComboBox.GetCurSel();
	GetDlgItemText (IDC_ET_EDIT_AFT, m_Output_tmp);

	if ((m_Output_CP != CB_ERR) && (m_Output_tmp.Compare(_T("")) == 0)) {
		AfxMessageBox (_T("텍스트와 코드페이지를 입력해주세요.")); return;
	}
	//***출력할 텍스트를 확인하고 빈 문자열이 아니면 ok한다

	CDialogEx::OnOK();
}