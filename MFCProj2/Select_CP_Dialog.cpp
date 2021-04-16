// Select_CP_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Select_CP_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Functions.h"

extern TCHAR *CodePageMenu[];

// Select_CP_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Select_CP_Dialog, CDialogEx)

Select_CP_Dialog::Select_CP_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Select_CP_Dialog::IDD, pParent)
{

}

Select_CP_Dialog::~Select_CP_Dialog()
{
}

void Select_CP_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCP_RADIO1, m_CP_Radio1);
	DDX_Control(pDX, IDC_SCP_RADIO2, m_CP_Radio2);
}


BEGIN_MESSAGE_MAP(Select_CP_Dialog, CDialogEx)
END_MESSAGE_MAP()


// Select_CP_Dialog 메시지 처리기입니다.


BOOL Select_CP_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	
	SetDlgItemText (IDC_SCP_RADIO1, CodePageMenu[KOR_CODE]);
	SetDlgItemText (IDC_SCP_RADIO2, CodePageMenu[JAP_CODE]);
	//*텍스트 세팅

	m_CP_Radio1.SetCheck(BST_UNCHECKED);
	m_CP_Radio2.SetCheck(BST_CHECKED);
	//*라디오 세팅

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void Select_CP_Dialog::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

//	CDialogEx::OnCancel();
	//*캔슬 동작은 무시한다
}


void Select_CP_Dialog::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	
	if (m_CP_Radio1.GetCheck() == BST_CHECKED) { CP_Code = KOR_CODE; }
	else if (m_CP_Radio2.GetCheck() == BST_CHECKED) { CP_Code = JAP_CODE; }
	//*변수를 세팅하고 나가면 된다

	CDialogEx::OnOK();
}
