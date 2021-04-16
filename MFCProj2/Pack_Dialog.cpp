// Pack_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Pack_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Pack.h"
//*병합할 수 있도록 병합 관련 함수가 선언된 파일 포함

extern CString Wolf_File_ListName;
extern CString Wolf_File_List;
extern TCHAR *CodePageMenu[];

CString Wolf_File_List_Backup;
//*백업용

// Pack_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Pack_Dialog, CDialogEx)

Pack_Dialog::Pack_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Pack_Dialog::IDD, pParent)
	, Pack_Thread(NULL)
{

}

Pack_Dialog::~Pack_Dialog()
{
}

void Pack_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_P_EDIT1, m_Wolf_Package_Name);
	DDX_Control(pDX, IDC_P_TEXT, m_Pack_Print_String);
	DDX_Control(pDX, IDC_P_PROGRESS1, m_Pack_Progress);
	DDX_Control(pDX, IDC_P_CP_RADIO1, m_CP_Radio1);
	DDX_Control(pDX, IDC_P_CP_RADIO2, m_CP_Radio2);
	DDX_Control(pDX, IDC_P_TP_RADIO1, m_Type_Radio1);
	DDX_Control(pDX, IDC_P_TP_RADIO2, m_Type_Radio2);
	DDX_Control(pDX, IDC_P_TP_RADIO3, m_Type_Radio3);
	DDX_Control(pDX, IDC_P_TP_RADIO4, m_Type_Radio4);
	DDX_Control(pDX, IDC_P_TP_RADIO5, m_Type_Radio5);
}


BEGIN_MESSAGE_MAP(Pack_Dialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &Pack_Dialog::OnBnClickedPack)
END_MESSAGE_MAP()


// Pack_Dialog 메시지 처리기입니다.


BOOL Pack_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetDlgItemText (IDC_P_CP_RADIO1, CodePageMenu[KOR_CODE]);
	SetDlgItemText (IDC_P_CP_RADIO2, CodePageMenu[JAP_CODE]);
	//*텍스트 세팅

	SetDlgItemText (IDC_P_EDIT1, Wolf_File_ListName.Left(Wolf_File_ListName.GetLength() - 4) + _T(".wolf"));
	//*디폴트 이름 세팅(리스트 이름에서 .txt를 빼고 .wolf를 붙이자)

	m_CP_Radio1.SetCheck(BST_CHECKED);
	m_CP_Radio2.SetCheck(BST_UNCHECKED);

	m_Type_Radio1.SetCheck(BST_CHECKED);
	m_Type_Radio2.SetCheck(BST_UNCHECKED);
	m_Type_Radio3.SetCheck(BST_UNCHECKED);
	m_Type_Radio4.SetCheck(BST_UNCHECKED);
	m_Type_Radio5.SetCheck(BST_UNCHECKED);
	//*라디오 세팅

	m_Pack_Progress.SetRange32 (0, 100);
	//*프로그레스 바 세팅

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void Pack_Dialog::OnBnClickedPack()
{
	//***Pack***
	CString Pack_Name;

	GetDlgItemText (IDC_P_EDIT1, Pack_Name);
	if (Pack_Name.Compare(_T("")) == 0) {
		AfxMessageBox (_T("Wolf 파일명을 입력하세요."));
		return;
	}
	//*입력한 파일명 얻어내기

	Wolf_File_List_Backup = Wolf_File_List;
	//*리스트 백업

	Pack_Thread = AfxBeginThread (Pack_Func, this);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDC_P_EDIT1)->EnableWindow(FALSE);
	//*분해 스레드 가동, Pack 버튼, 입력창 비활성화

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


UINT Pack_Dialog::Pack_Func (LPVOID lParam)
{
	Pack_Dialog* dlg;
	int DXA_Code, LANG_Code;
	//*wolf 파일 버전, 지정된 언어

	dlg = (Pack_Dialog*)lParam;
	//*객체 얻기

	CString Pack_Name;
	dlg->GetDlgItemText (IDC_P_EDIT1, Pack_Name);
	//*합성할 이름 얻기

	if (dlg->m_CP_Radio1.GetCheck() == BST_CHECKED) { LANG_Code = KOR_CODE; }
	else if (dlg->m_CP_Radio2.GetCheck() == BST_CHECKED) { LANG_Code = JAP_CODE; }
	//*어느 언어를 지정하여 분해할 것인가.
	
	if (dlg->m_Type_Radio1.GetCheck() == BST_CHECKED) { DXA_Code = DXA_LOW; }
	else if (dlg->m_Type_Radio2.GetCheck() == BST_CHECKED) { DXA_Code = DXA_220; }
	else if (dlg->m_Type_Radio3.GetCheck() == BST_CHECKED) { DXA_Code = DXA_330; }
	else if (dlg->m_Type_Radio4.GetCheck() == BST_CHECKED) { DXA_Code = DXA_THMK; }
	else if (dlg->m_Type_Radio5.GetCheck() == BST_CHECKED) { DXA_Code = DXA_THMK_64; }
	//*어느 버전으로 패킹할 것인가.

	if ((DXA_Code == DXA_330) || DXA_Code == DXA_THMK_64) { creating_64bit (dlg, DXA_Code, LANG_Code); }
	else { creating (dlg, DXA_Code, LANG_Code); }
	//*분해 & 리스트 갱신 & 프로그레스 갱신*

	CString Check;
	dlg->GetDlgItemText (IDC_P_TEXT, Check);
	if (Check.Compare(_T("Packing Finished")) == 0) {
		AfxMessageBox (Pack_Name + _T(" 합성 완료!"));
	}

	dlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
	dlg->GetDlgItem(IDC_P_EDIT1)->EnableWindow(TRUE);
	//*버튼 활성화

	Wolf_File_List = Wolf_File_List_Backup;
	//*리스트 복구

	return 0;
}


void Pack_Dialog::OnCancel()
{
	//***Cancel***

	DWORD ExitCode;

	if (Pack_Thread != NULL) {
		GetExitCodeThread (Pack_Thread->m_hThread, &ExitCode);
		if (ExitCode == STILL_ACTIVE) {
			
			Pack_Thread->SuspendThread();
			int ret = AfxMessageBox(_T("합성을 멈추겠습니까?"), MB_YESNO);
			//*만일 파일 분해 스레드가 돌아가고 있다면 일시정지시킨 후 계속할 거냐 묻는다.

			if (ret == IDYES) {
				TerminateThread (Pack_Thread->m_hThread, 0);
				CloseHandle (Pack_Thread->m_hThread);
				Wolf_File_List = Wolf_File_List_Backup;
				//*취소할 때는 백업한 리스트를 이용해 복구해야 한다. 이름은 변경없음
			}
			else {
				Pack_Thread->ResumeThread();
				return;
			}
			//*계속한다면 다시 재개 후 돌아가고, 그만둔다면 그대로 스레드를 종료시킨 후 OnCancel을 호출한다

		}
	}

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}