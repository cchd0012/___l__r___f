// File_Or_Text_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "File_Or_Text_Dialog.h"
#include "afxdialogex.h"


// File_Or_Text_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(File_Or_Text_Dialog, CDialogEx)

File_Or_Text_Dialog::File_Or_Text_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(File_Or_Text_Dialog::IDD, pParent)
{
	m_FN_Radio.SetCheck (BST_CHECKED);
	m_TE_Radio.SetCheck (BST_UNCHECKED);
	Type = 0;
	//*버튼 초기화. 일반적으로는 파일 이름일 때 띄우므로 파일로 설정
}

File_Or_Text_Dialog::~File_Or_Text_Dialog()
{
}

void File_Or_Text_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FN_RADIO, m_FN_Radio);
	DDX_Control(pDX, IDC_TE_RADIO, m_TE_Radio);
}


BEGIN_MESSAGE_MAP(File_Or_Text_Dialog, CDialogEx)
END_MESSAGE_MAP()


// File_Or_Text_Dialog 메시지 처리기입니다.


void File_Or_Text_Dialog::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (m_FN_Radio.GetCheck() == BST_CHECKED) { Type = 1; }
	else if (m_TE_Radio.GetCheck() == BST_CHECKED) { Type = 2; }
	//*선택된 버튼에 따라 Type을 설정

	CDialogEx::OnOK();
}