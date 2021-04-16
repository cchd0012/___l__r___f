// Extract_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Extract_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Extract.h"
//*분해할 수 있도록 분해 관련 함수가 선언된 파일 포함

extern CString Wolf_File_ListName;
extern CString Wolf_File_List;
extern TCHAR *CodePageMenu[];

// Extract_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Extract_Dialog, CDialogEx)


Extract_Dialog::Extract_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Extract_Dialog::IDD, pParent)
	, Wolf_Full_Path(_T(""))
	, Wolf_File_Name(_T(""))
	, Wolf_File_Title(_T(""))
	, Extract_Thread(NULL)
{

}

Extract_Dialog::~Extract_Dialog()
{
}

void Extract_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_E_EDIT1, Wolf_Full_Path);
	DDX_Control(pDX, IDC_E_TEXT, m_Extract_Print_String);
	DDX_Control(pDX, IDC_E_CP_RADIO1, m_CP_Radio1);
	DDX_Control(pDX, IDC_E_CP_RADIO2, m_CP_Radio2);
	DDX_Control(pDX, IDC_E_PROGRESS1, m_Extract_Progress);
}


BEGIN_MESSAGE_MAP(Extract_Dialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &Extract_Dialog::OnBnClickedSelectButton)
	ON_BN_CLICKED(IDOK, &Extract_Dialog::OnBnClickedExtract)
END_MESSAGE_MAP()


// Extract_Dialog 메시지 처리기입니다.


void Extract_Dialog::OnBnClickedSelectButton()
{
	TCHAR szFilter[] = _T("Wolf Package File (*.wolf) |*.wolf|DXA Files(*.dxa) |*.dxa||");
	CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilter);

	if (dlg.DoModal() == IDOK) {
		UpdateData(TRUE);
		Wolf_Full_Path = dlg.GetPathName();
		Wolf_File_Name = dlg.GetFileName();
		Wolf_File_Title = dlg.GetFileTitle();
		UpdateData(FALSE);
	}
	//* wolf_Full_path 갱신

	//***GetFileName : 확장자를 포함한 파일의 이름
	//***GetFileTitle : 확장자를 제외한 파일의 이름
	//***GetFolderPath : 선택한 파일의 폴더 경로
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Extract_Dialog::OnBnClickedExtract()
{
	if (Wolf_Full_Path.Compare(_T("")) == 0) {
		AfxMessageBox (_T("Wolf 파일을 선택하세요."));
		return;
	}
	//*파일명 체크, 파일명이 없으면 가동하지 않고 그냥 돌아감

	int DXA_Code = GetWolfFileVersion (Wolf_Full_Path);
	//*wolf 파일 버전 확인

	if (DXA_Code == -2) {
		AfxMessageBox (_T("해당 Wolf 파일이 없습니다."));
		return;
	}
	else if (DXA_Code == -1) {
		AfxMessageBox (_T("올바른 Wolf 파일이 아닙니다."));
		return;
	}
	//*파일 유형 체크, 파일이 없거나 파일 유형이 올바르지 않다면 돌아감

	Extract_Thread = AfxBeginThread (Extract_Func, this);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	//*분해 스레드 가동, Extract 버튼 비활성화

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


UINT Extract_Dialog::Extract_Func (LPVOID lParam)
{
	Extract_Dialog* dlg;
	int DXA_Code, LANG_Code;
	//*wolf 파일 버전, 지정된 언어

	Wolf_File_ListName = _T("");
	Wolf_File_List.Empty();
	//*리스트 초기화

	dlg = (Extract_Dialog*)lParam;
	//*객체 얻기

	DXA_Code = GetWolfFileVersion (dlg->Wolf_Full_Path);
	//*wolf 파일 버전 재확인. 정확도를 위해서.

	if (dlg->m_CP_Radio1.GetCheck() == BST_CHECKED) { LANG_Code = KOR_CODE; }
	else if (dlg->m_CP_Radio2.GetCheck() == BST_CHECKED) { LANG_Code = JAP_CODE; }
	//*어느 언어를 지정하여 분해할 것인가.

	if ((DXA_Code == DXA_330) || (DXA_Code == DXA_THMK_64)) { extracting_64bit (dlg, DXA_Code, LANG_Code); }
	else { extracting (dlg, DXA_Code, LANG_Code); }
	//*분해 & 리스트 갱신 & 프로그레스 갱신*

	CString Check;
	dlg->GetDlgItemText (IDC_E_TEXT, Check);
	if (Check.Compare(_T("Extract Finished")) == 0) {
		CString tt = dlg->Wolf_File_Name + _T(" 분해 완료! (");
		if (DXA_Code == DXA_LOW) { tt += _T("type 1)"); }
		else if (DXA_Code == DXA_220) { tt += _T("type 2)"); }
		else if (DXA_Code == DXA_330) { tt += _T("type 3)"); }
		else if (DXA_Code == DXA_THMK) { tt += _T("type th_l_2)"); }
		else if (DXA_Code == DXA_THMK_64) { tt += _T("type th_l_2_64bit)"); }
		else { tt += _T("type Unknown)"); }
		AfxMessageBox (tt);
	}

	dlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
	//*버튼 활성화

	return 0;
}


BOOL Extract_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetDlgItemText (IDC_E_TEXT, _T("Extracting : "));
	SetDlgItemText (IDC_E_CP_RADIO1, CodePageMenu[KOR_CODE]);
	SetDlgItemText (IDC_E_CP_RADIO2, CodePageMenu[JAP_CODE]);
	//*텍스트 세팅

	m_CP_Radio1.SetCheck(BST_UNCHECKED);
	m_CP_Radio2.SetCheck(BST_CHECKED);
	//*라디오 세팅

	m_Extract_Progress.SetRange32 (0, 100);
	//*프로그레스 바 세팅

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void Extract_Dialog::OnCancel()
{
	DWORD ExitCode;

	if (Extract_Thread != NULL) {
		GetExitCodeThread (Extract_Thread->m_hThread, &ExitCode);
		if (ExitCode == STILL_ACTIVE) {
			
			Extract_Thread->SuspendThread();
			int ret = AfxMessageBox(_T("분해를 멈추겠습니까?"), MB_YESNO);
			//*만일 파일 분해 스레드가 돌아가고 있다면 일시정지시킨 후 계속할 거냐 묻는다.

			if (ret == IDYES) {
				TerminateThread (Extract_Thread->m_hThread, 0);
				CloseHandle (Extract_Thread->m_hThread);
				Wolf_File_ListName = _T("");
				Wolf_File_List.Empty();
				//*취소할 때는 리스트를 초기화해야 한다
			}
			else {
				Extract_Thread->ResumeThread();
				return;
			}
			//*계속한다면 다시 재개 후 돌아가고, 그만둔다면 그대로 스레드를 종료시킨 후 OnCancel을 호출한다

		}
	}

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}