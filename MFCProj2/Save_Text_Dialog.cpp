// Save_Text_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Save_Text_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Text.h"


// Save_Text_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Save_Text_Dialog, CDialogEx)

Save_Text_Dialog::Save_Text_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Save_Text_Dialog::IDD, pParent)
{

}

Save_Text_Dialog::~Save_Text_Dialog()
{
}

void Save_Text_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_Save_Text_Progress);
}


BEGIN_MESSAGE_MAP(Save_Text_Dialog, CDialogEx)
END_MESSAGE_MAP()


// Save_Text_Dialog 메시지 처리기입니다.


BOOL Save_Text_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	m_Save_Text_Progress.SetRange32 (0, 100);
	//*프로그레스 초기화

	Save_Text_Thread = AfxBeginThread (Save_Text_Func, this);
	//*프로그레스 스레드 구동

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL Save_Text_Dialog::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE)) { return true; }
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN)) { return true; }
	//*Enter, ESC를 무력화시킨다

	if (m_Save_Text_Progress.GetPos() == 100) { ::SendMessage (m_hWnd, WM_CLOSE, NULL, NULL); }

	return CDialogEx::PreTranslateMessage(pMsg);
}


UINT Save_Text_Dialog::Save_Text_Func (LPVOID lParam)
{
	Save_Text_Dialog *dlg = (Save_Text_Dialog*)lParam;
	//*객체 얻기

	CString Dir_Path = dlg->m_FP_tmp;
	//*디렉토리명 얻기

	if (Save_Text_To_Directory_Data (dlg, Dir_Path)== -1) { 
		dlg->m_Save_Text_Progress.SetPos (100);
	}
	//*저장 함수 호출

//	AfxMessageBox (_T("텍스트 저장 완료!"));

	return 0;
}