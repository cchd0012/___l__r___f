// Autotrans_Papago_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Autotrans_Papago_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Auto_Papago_Translate.h"
// Autotrans_Papago_Dialog 대화 상자입니다.

extern DIR_TXT_IDX m_Text_Idx_In_Dir;

IMPLEMENT_DYNAMIC(Autotrans_Papago_Dialog, CDialogEx)

Autotrans_Papago_Dialog::Autotrans_Papago_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Autotrans_Papago_Dialog::IDD, pParent)
{

}

Autotrans_Papago_Dialog::~Autotrans_Papago_Dialog()
{
}

void Autotrans_Papago_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_ORG, m_Src_Language_List);
	DDX_Control(pDX, IDC_COMBO_ORG2, m_Dst_Language_List);
	DDX_Control(pDX, IDC_TRANS_PROGRESS, m_Translate_Progress);
	DDX_Control(pDX, IDC_COMBO_TRANS_FILE, m_Text_File_List);
	DDX_Control(pDX, IDC_START_IDX, m_Start_Idx_Str);
	DDX_Control(pDX, IDC_END_IDX, m_End_Idx_Str);
}


BEGIN_MESSAGE_MAP(Autotrans_Papago_Dialog, CDialogEx)
	ON_BN_CLICKED(ID_TRANSLATE, &Autotrans_Papago_Dialog::OnBnClickedTranslate)
	ON_CBN_SELCHANGE(IDC_COMBO_TRANS_FILE, &Autotrans_Papago_Dialog::OnCbnSelchangeComboTransFile)
END_MESSAGE_MAP()


// Autotrans_Papago_Dialog 메시지 처리기입니다.


BOOL Autotrans_Papago_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_Src_Language_List.AddString (_T("en"));
	m_Src_Language_List.AddString (_T("ja"));
	m_Src_Language_List.AddString (_T("ko"));
	m_Src_Language_List.SetCurSel (1);

	m_Dst_Language_List.AddString (_T("en"));
	m_Dst_Language_List.AddString (_T("ja"));
	m_Dst_Language_List.AddString (_T("ko"));
	m_Dst_Language_List.SetCurSel (2);
	// 번역 언어 목록 추가, 기본적으로 셀렉트된 식으로 하면 됨

	progressing_file_idx = 0;
	//*(중간에 나갈 시)진행 중이었던 파일 인덱스 세팅

	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {
		if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt != 0) {
			TCHAR *t = _tcsstr(m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, _T("data/"));
			if (t == NULL){ t = _tcsstr(m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, _T("data\\")); }
			if (t == NULL){ t = _tcsstr(m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, _T("Data/")); }
			if (t == NULL){ t = _tcsstr(m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, _T("Data\\")); }
			m_Text_File_List.AddString (t);
		}
	}
	CString _t;
	m_Text_File_List.SetCurSel (0);
	_t.Format(_T("%06d"), 0); 
	m_Start_Idx_Str.SetWindowTextW (_t);
	s_idx = 0;
	_t.Format(_T("%06d"), (m_Text_Idx_In_Dir.File_Idx_Arr[0].Text_Cnt-1)); 
	m_End_Idx_Str.SetWindowTextW (_t);
	e_idx = (m_Text_Idx_In_Dir.File_Idx_Arr[0].Text_Cnt-1);
	//*텍스트가 들어 있는 파일 리스트 및 범위 세팅

	m_Translate_Progress.SetRange32 (0, 100);
	//*프로그레스 바 세팅

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


UINT Autotrans_Papago_Dialog::Translate_Func (LPVOID lParam)
{
	Autotrans_Papago_Dialog* dlg;

	dlg = (Autotrans_Papago_Dialog*)lParam;
	//*객체 얻기

	CString srcLang, dstLang, FilePath, startIdx, endIdx;
	dlg->m_Src_Language_List.GetLBText (dlg->m_Src_Language_List.GetCurSel(), srcLang);
	dlg->m_Dst_Language_List.GetLBText (dlg->m_Dst_Language_List.GetCurSel(), dstLang);
	dlg->m_Text_File_List.GetLBText (dlg->m_Text_File_List.GetCurSel(), FilePath);
	Papago_Translate (dlg, FilePath.GetBuffer(), srcLang.GetBuffer(), dstLang.GetBuffer(), 
		dlg->s_idx, dlg->e_idx);
	//*번역 진행

	CString Check;
	dlg->GetDlgItemText (IDC_TRANS_PROG_TEXT, Check);
	if (Check.Compare(_T("Translating Finished")) == 0) {
		AfxMessageBox (FilePath + _T(" Papago 자동번역 완료!"));
	}

	dlg->GetDlgItem(ID_TRANSLATE)->EnableWindow(TRUE);
	//*버튼 활성화

	return 0;
}
//번역 스레드


void Autotrans_Papago_Dialog::OnOK()
{
	//*엔터키를 눌렀을 때 좀 신경써주면 될듯?
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}


void Autotrans_Papago_Dialog::OnCancel()
{
	//*Cancel 버튼을 눌렀을 때

	DWORD ExitCode;

	if (Translate_Thread != NULL) {
		GetExitCodeThread (Translate_Thread->m_hThread, &ExitCode);
		if (ExitCode == STILL_ACTIVE) {
			
			Translate_Thread->SuspendThread();
			int ret = AfxMessageBox(_T("번역을 멈추겠습니까?"), MB_YESNO);
			//*만일 파일 분해 스레드가 돌아가고 있다면 일시정지시킨 후 계속할 거냐 묻는다.

			if (ret == IDYES) {
				TerminateThread (Translate_Thread->m_hThread, 0);
				CloseHandle (Translate_Thread->m_hThread);
				//*취소할 때는 그냥 현재까지 번역한 만큼만 저장하도록 한다
			}
			else {
				Translate_Thread->ResumeThread();
				return;
			}
			//*계속한다면 다시 재개 후 돌아가고, 그만둔다면 그대로 스레드를 종료시킨 후 OnCancel을 호출한다

		}
	}


	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void Autotrans_Papago_Dialog::OnBnClickedTranslate()
{
	//*Translate 버튼을 눌렀을 때

	if (m_Src_Language_List.GetCurSel() == m_Dst_Language_List.GetCurSel()) {
		AfxMessageBox (_T("원본 언어와 번역 언어가 같으면 안 됩니다."));
		return;
	}
	//*시작어와 번역어가 똑같으면 안 된다

	CString FilePath;
	m_Text_File_List.GetLBText (m_Text_File_List.GetCurSel(), FilePath);
	unsigned int File_Idx = 0;
	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {
		if (_tcsstr (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, FilePath) != NULL) {
			File_Idx = i; break;
		}
	}
	//*번역할 파일의 인덱스 구하기
	
	CString startIdx, endIdx;
	m_Start_Idx_Str.GetWindowTextW (startIdx);
	for (int i = 0;i < startIdx.GetLength();i++) {
		if ((startIdx[i] < '0') || (startIdx[i] > '9')) {
			AfxMessageBox (_T("올바른 시작 인덱스 값이 아닙니다."));
			return;
		}
	}
	m_End_Idx_Str.GetWindowTextW (endIdx);
	for (int i = 0;i < endIdx.GetLength();i++) {
		if ((endIdx[i] < '0') || (endIdx[i] > '9')) {
			AfxMessageBox (_T("올바른 종결 인덱스 값이 아닙니다."));
			return;
		}
	}
	//*시작/종결 인덱스의 값이 올바른지 확인

	s_idx = _ttoi(startIdx);
	e_idx = _ttoi(endIdx);
	if (e_idx < s_idx) {
		AfxMessageBox (_T("종결 인덱스는 시작 인덱스보다 작을 수 없습니다."));
		return;
	}
	//*시작/종결 인덱스의 범위 확인

	if (s_idx < 0) { 
		AfxMessageBox (_T("시작 인덱스 값이 0보다 작습니다.\n0으로 맞춰집니다."));
	}
	if (e_idx >= m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Text_Cnt) { 
		AfxMessageBox (_T("종결 인덱스 값이 파일 범위를 벗어납니다.\n최대치로 맞춰집니다."));
		e_idx = (m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Text_Cnt - 1);
	}
	//*시작/종결 인덱스가 텍스트 범위를 벗어나면 알아서 맞추기

	Translate_Thread = AfxBeginThread (Translate_Func, this);
	GetDlgItem(ID_TRANSLATE)->EnableWindow(FALSE);
	//*분해 스레드 가동, Translate 버튼 비활성화

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Autotrans_Papago_Dialog::OnCbnSelchangeComboTransFile()
{
	CString FilePath;
	m_Text_File_List.GetLBText (m_Text_File_List.GetCurSel(), FilePath);
	unsigned int File_Idx = 0;
	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {
		if (_tcsstr (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, FilePath) != NULL) {
			File_Idx = i; break;
		}
	}
	//*번역할 파일의 인덱스 구하기
	
	CString _t;
	_t.Format(_T("%06d"), 0); 
	m_Start_Idx_Str.SetWindowTextW (_t);
	s_idx = 0;
	_t.Format(_T("%06d"), (m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Text_Cnt-1)); 
	m_End_Idx_Str.SetWindowTextW (_t);
	e_idx = (m_Text_Idx_In_Dir.File_Idx_Arr[0].Text_Cnt-1);
	//*텍스트가 들어 있는 파일 리스트 및 범위 세팅

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
