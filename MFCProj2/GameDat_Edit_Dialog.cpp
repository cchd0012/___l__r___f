// GameDat_Edit_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "GameDat_Edit_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Functions.h"

extern TCHAR *CodePageMenu[];

// GameDat_Edit_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(GameDat_Edit_Dialog, CDialogEx)

GameDat_Edit_Dialog::GameDat_Edit_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(GameDat_Edit_Dialog::IDD, pParent)
{

}

GameDat_Edit_Dialog::~GameDat_Edit_Dialog()
{
}

void GameDat_Edit_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GD_EDIT1, m_Game_Title);
	DDX_Control(pDX, IDC_GD_EDIT2, m_Font1);
	DDX_Control(pDX, IDC_GD_EDIT3, m_Font2);
	DDX_Control(pDX, IDC_GD_EDIT4, m_Font3);
	DDX_Control(pDX, IDC_GD_EDIT5, m_Font4);
	DDX_Control(pDX, IDC_GD_RADIO1, m_GD_CPRadio1);
	DDX_Control(pDX, IDC_GD_RADIO2, m_GD_CPRadio2);
}


BEGIN_MESSAGE_MAP(GameDat_Edit_Dialog, CDialogEx)
END_MESSAGE_MAP()


// GameDat_Edit_Dialog 메시지 처리기입니다.


BOOL GameDat_Edit_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetDlgItemText (IDC_GD_EDIT1, Game_Title);
	SetDlgItemText (IDC_GD_EDIT2, Font1);
	SetDlgItemText (IDC_GD_EDIT3, Font2);
	SetDlgItemText (IDC_GD_EDIT4, Font3);
	SetDlgItemText (IDC_GD_EDIT5, Font4);
	
	SetDlgItemText (IDC_GD_RADIO1, CodePageMenu[KOR_CODE]);
	SetDlgItemText (IDC_GD_RADIO2, CodePageMenu[JAP_CODE]);
	//*텍스트 세팅
	
	m_GD_CPRadio1.SetCheck(BST_CHECKED);
	m_GD_CPRadio2.SetCheck(BST_UNCHECKED);
	//*라디오 세팅

	if (type == DXA_330) { SetWindowText (_T("Edit 'Game.dat' (type 2/3)")); }
	else { SetWindowText (_T("Edit 'Game.dat' (type 1)")); }
	//*캡션 세팅

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void GameDat_Edit_Dialog::OnOK()
{

	GetDlgItemText (IDC_GD_EDIT1, Game_Title);
	GetDlgItemText (IDC_GD_EDIT2, Font1);
	GetDlgItemText (IDC_GD_EDIT3, Font2);
	GetDlgItemText (IDC_GD_EDIT4, Font3);
	GetDlgItemText (IDC_GD_EDIT5, Font4);
	//*문자열 얻기

	if (m_GD_CPRadio1.GetCheck() == BST_CHECKED) { code = KOR_CODE; }
	else if (m_GD_CPRadio2.GetCheck() == BST_CHECKED) { code = JAP_CODE; }
	//*버튼 클릭상태 얻기

	if ((Game_Title.GetLength() == 0) || (Font1.GetLength() == 0)) {
		AfxMessageBox (_T("게임 타이틀 혹은 첫 번째 폰트가 비어있으면 안 됩니다."));
		return;
	}
	//*게임 타이틀이나 첫번째 폰트가 비어있으면 안 된다.

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}
