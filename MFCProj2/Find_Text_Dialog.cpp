// Find_Text_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Find_Text_Dialog.h"
#include "afxdialogex.h"


// Find_Text_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Find_Text_Dialog, CDialogEx)

Find_Text_Dialog::Find_Text_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Find_Text_Dialog::IDD, pParent)
{

}

Find_Text_Dialog::~Find_Text_Dialog()
{
}

void Find_Text_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FT_EDIT1, m_Find_String);
	DDX_Control(pDX, IDC_FT_RADIO1, m_FT_Up_Radio);
	DDX_Control(pDX, IDC_FT_RADIO2, m_FT_Down_Radio);
}


BEGIN_MESSAGE_MAP(Find_Text_Dialog, CDialogEx)
END_MESSAGE_MAP()


// Find_Text_Dialog 메시지 처리기입니다.


BOOL Find_Text_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_FT_Up_Radio.SetCheck (BST_UNCHECKED);
	m_FT_Down_Radio.SetCheck (BST_CHECKED);
	//*라디오 세팅

	Direction = DOWN_DIRECTION;
	//*방향 세팅

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void Find_Text_Dialog::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	GetDlgItemText (IDC_FT_EDIT1, Find_Str);
	if (Find_Str.GetLength() == 0) {
		AfxMessageBox (_T("탐색할 텍스트는 길이가 1 이상이어야 합니다."));
		return;
	}
	//*찾는 문자열이 빈 문자열이면 안된다

	if (m_FT_Up_Radio.GetCheck() == BST_CHECKED) { Direction = UP_DIRECTION; }
	else if (m_FT_Down_Radio.GetCheck() == BST_CHECKED) { Direction = DOWN_DIRECTION; }
	else { Direction = NON_DIRECTION; }
	//*방향 세팅

	CDialogEx::OnOK();
}