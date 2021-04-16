// Change_FDName_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Change_FDName_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Functions.h"


// Change_FDName_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Change_FDName_Dialog, CDialogEx)

Change_FDName_Dialog::Change_FDName_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Change_FDName_Dialog::IDD, pParent)
{

}

Change_FDName_Dialog::~Change_FDName_Dialog()
{
}

void Change_FDName_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CFD_EDIT1, m_Input_Name);
	DDX_Control(pDX, IDC_CFD_EDIT2, m_Output_Name);
}


BEGIN_MESSAGE_MAP(Change_FDName_Dialog, CDialogEx)
END_MESSAGE_MAP()


// Change_FDName_Dialog 메시지 처리기입니다.


BOOL Change_FDName_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetDlgItemText (IDC_CFD_EDIT1, Input_WStr);
	SetDlgItemText (IDC_CFD_EDIT2, Output_WStr);
	//*텍스트 세팅

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void Change_FDName_Dialog::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	GetDlgItemText (IDC_CFD_EDIT2, Output_WStr);

	if (Output_WStr.GetLength() == 0) {
		AfxMessageBox (_T("변환할 폴더명을 입력하세요")); return;
	}

	CString sf; char *tmp; wchar_t *tmp_u;
	if (code == KOR_CODE) {
		tmp = U2K (Output_WStr.GetBuffer()); tmp_u = K2U (tmp); sf = tmp_u;
	}
	else if (code == JAP_CODE) {
		tmp = U2J (Output_WStr.GetBuffer()); tmp_u = J2U (tmp); sf = tmp_u;
	}
	free (tmp); free (tmp_u);

	if (Output_WStr != sf) {
		AfxMessageBox (_T("문자열이 언어코드와 호환되지 않습니다")); return;
	}
	
	for (unsigned int i = 0;i < (unsigned int)Output_WStr.GetLength();i++) {
		if (IS_NOFILE_CHAR(Output_WStr[i])) {
			if ((Meta_type == DIR) && ((Output_WStr[i] == '/') || (Output_WStr[i] == '\\'))) { continue; }
			AfxMessageBox (_T("해당 문자열은 파일/폴더명이 될 수 없습니다.")); return;
		}
	}

	if (Meta_type == DIR && 
		((Output_WStr[Output_WStr.GetLength() - 1] != '/') && (Output_WStr[Output_WStr.GetLength() - 1] != '\\'))) {
			AfxMessageBox (_T("폴더 마지막 문자는 슬래시여야 합니다.")); return;
	}
	//*디렉토리면 맨 마지막에 '/'나 '\\'가 와야 한다

	CDialogEx::OnOK();
}


void Change_FDName_Dialog::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (ShouldChange){
		int ret = AfxMessageBox (_T("문자열을 변경하지 않겠습니까?\n폴더/파일명을 바꾸지 않으면 실행시 에러가 날 수 있습니다."), 
			MB_YESNO);
		if (ret == IDNO) { return; }
	}

	CDialogEx::OnCancel();
}
