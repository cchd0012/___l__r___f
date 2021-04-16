// Check_FDName_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Check_FDName_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Text.h"


// Check_FDName_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Check_FDName_Dialog, CDialogEx)

Check_FDName_Dialog::Check_FDName_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Check_FDName_Dialog::IDD, pParent)
{

}

Check_FDName_Dialog::~Check_FDName_Dialog()
{
}

void Check_FDName_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_FD_TEXT, m_Check_FDName_Print);
	DDX_Control(pDX, IDC_CHK_FD_PROGRESS, m_Check_FDName_Progress);
}


BEGIN_MESSAGE_MAP(Check_FDName_Dialog, CDialogEx)
END_MESSAGE_MAP()


// Check_FDName_Dialog 메시지 처리기입니다.


BOOL Check_FDName_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	m_Check_FDName_Progress.SetRange32 (0, 100);
	//*프로그레스 초기화

	Save_Text_Thread = AfxBeginThread (Check_FDName_Func, this);
	//*프로그레스 스레드 구동

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL Check_FDName_Dialog::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE)) { return true; }
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN)) { return true; }
	//*Enter, ESC를 무력화시킨다

	if (m_Check_FDName_Progress.GetPos() == 100) { ::SendMessage (m_hWnd, WM_CLOSE, NULL, NULL); }

	return CDialogEx::PreTranslateMessage(pMsg);
}


UINT Check_FDName_Dialog::Check_FDName_Func (LPVOID lParam)
{
	Check_FDName_Dialog *dlg = (Check_FDName_Dialog*)lParam;
	//*객체 얻기

	Check_Filename_For_Codepage (dlg, dlg->code);
	//*체크 함수 호출

	AfxMessageBox (_T("파일/폴더명 체크 완료!"));

	return 0;
}