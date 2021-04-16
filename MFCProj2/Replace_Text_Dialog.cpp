// Replace_Text_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Replace_Text_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Functions.h"

extern TCHAR *CodePageMenu[];


// Replace_Text_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Replace_Text_Dialog, CDialogEx)

Replace_Text_Dialog::Replace_Text_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Replace_Text_Dialog::IDD, pParent)
{

}

Replace_Text_Dialog::~Replace_Text_Dialog()
{
}

void Replace_Text_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RPL_EDIT1, m_Replace_Text_Find);
	DDX_Control(pDX, IDC_RPL_EDIT2, m_Replace_Text_Change);
	DDX_Control(pDX, IDC_RP_CP_RADIO1, m_CP_Radio1);
	DDX_Control(pDX, IDC_RP_CP_RADIO2, m_CP_Radio2);
	DDX_Control(pDX, IDC_RP_CP_RADIO3, m_CP_Radio3);
	DDX_Control(pDX, IDC_CHK_REP_FN, m_RF_check);
}


BEGIN_MESSAGE_MAP(Replace_Text_Dialog, CDialogEx)
END_MESSAGE_MAP()


// Replace_Text_Dialog 메시지 처리기입니다.


BOOL Replace_Text_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetDlgItemText (IDC_RP_CP_RADIO1, _T("None"));
	SetDlgItemText (IDC_RP_CP_RADIO2, CodePageMenu[KOR_CODE]);
	SetDlgItemText (IDC_RP_CP_RADIO3, CodePageMenu[JAP_CODE]);
	//*텍스트 세팅
	
	m_CP_Radio1.SetCheck(BST_CHECKED);
	m_CP_Radio2.SetCheck(BST_UNCHECKED);
	m_CP_Radio3.SetCheck(BST_UNCHECKED);
	m_RF_check.SetCheck(BST_UNCHECKED);
	//*라디오 세팅

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void Replace_Text_Dialog::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	GetDlgItemText (IDC_RPL_EDIT1, strtofind);
	GetDlgItemText (IDC_RPL_EDIT2, strtochg);

	if (strtochg.Compare(_T("__ENTER__")) == 0) { strtochg = _T("\n"); }
	//*개행으로 바꿀 조건 추가

	if (m_CP_Radio1.GetCheck() == BST_CHECKED) { code = NON_CODE; }
	else if (m_CP_Radio2.GetCheck() == BST_CHECKED) { code = KOR_CODE; }
	else if (m_CP_Radio3.GetCheck() == BST_CHECKED) { code = JAP_CODE; }

	if (m_RF_check.GetCheck() == BST_CHECKED) { FCH = true; } else { FCH = false; }

	if (strtofind.GetLength() == 0) {
		AfxMessageBox (_T("찾을 문자열은 길이가 1 이상이어야 합니다."));
		return;
	}

	CDialogEx::OnOK();
}