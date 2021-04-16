// Load_Text_Dialog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCProj2.h"
#include "Load_Text_Dialog.h"
#include "Select_CP_Dialog.h"
#include "afxdialogex.h"

#include "Wolf\Wolf_Text.h"

extern CListCtrl *m_G_ListCtrl_Files;
extern CListCtrl *m_G_ListCtrl_Text;
extern DIR_TXT_IDX m_Text_Idx_In_Dir;
//*extern으로 리스트 변수를 얻어옴 



void Get_Fidx_Tidx (unsigned int Idx, unsigned int *fidx, unsigned int *tidx)
{
	unsigned int f_idx, t_idx = Idx;
	if (m_Text_Idx_In_Dir.File_Num == 0) { (*fidx) = (*tidx) = 0xFFFFFFFF; return; }
	for (f_idx = 0;f_idx < m_Text_Idx_In_Dir.File_Num;f_idx++) {
		if (m_Text_Idx_In_Dir.File_Idx_Arr[f_idx].Text_Cnt != 0) {
			if (t_idx >= (m_Text_Idx_In_Dir.File_Idx_Arr[f_idx].Text_Cnt + 1)) 
			{ t_idx -= (m_Text_Idx_In_Dir.File_Idx_Arr[f_idx].Text_Cnt + 1); }
			else { break; }
		}
	}
	(*fidx) = f_idx; (*tidx) = t_idx; return;
}
//*들어온 인자값(전역 idx)을 바탕으로 파일 인덱스와 텍스트 인덱스를 저장시켜주는 함수


int Get_List_Index (unsigned int fidx, unsigned int tidx)
{
	if (m_Text_Idx_In_Dir.File_Num <= fidx) { return -1; }
	if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Cnt <= tidx) { return -1; }

	int ret = 0;
	for (unsigned int i = 0;i < fidx;i++) {
		if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt != 0) {
			ret += ((int)m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt + 1);
		}
	}
	if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Cnt != 0) { ret += ((int)tidx + 1); }
	//+1이 붙는 이유는 파일 텍스트 헤더 때문임

	return ret;
}
//*들어온 인자값(파일 인덱스, 텍스트 인덱스)을 바탕으로 리스트 인덱스를 반환하는 함수


void Dir_Idx_Reset (DIR_TXT_IDX *dti)
{
	for (unsigned int i = 0;i < dti->File_Num;i++) {
		if (dti->File_Idx_Arr[i].Text_Cnt != 0) { free (dti->File_Idx_Arr[i].Text_Idx_Arr); }
	}
	if (dti->File_Num != 0) { free (dti->File_Idx_Arr); }
	//*텍스트 총괄 구조체 해제

	dti->File_Num = 0;
	dti->File_Idx_Arr = NULL;
	//*텍스트 총괄 구조체 초기화
}


void All_List_Item_Delete()
{
	if (m_G_ListCtrl_Files->GetItemCount() != 0) { m_G_ListCtrl_Files->DeleteAllItems(); }
	if (m_G_ListCtrl_Text->GetItemCount() != 0) { m_G_ListCtrl_Text->DeleteAllItems(); }
	//*리스트 내용 전부 삭제
}


void All_Text_Data_Reset ()
{
	Dir_Idx_Reset (&m_Text_Idx_In_Dir);
	//*m_Text_Idx_In_Dir 변수 초기화
}



// Load_Text_Dialog 대화 상자입니다.

IMPLEMENT_DYNAMIC(Load_Text_Dialog, CDialogEx)

Load_Text_Dialog::Load_Text_Dialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Load_Text_Dialog::IDD, pParent)
	, Load_Text_Thread(NULL)
{

}

Load_Text_Dialog::~Load_Text_Dialog()
{
}

void Load_Text_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCP_EDIT, m_FolderPath);
	DDX_Control(pDX, IDC_LT_PROGRESS1, m_Load_Text_Progress);
	DDX_Control(pDX, IDC_LT_TEXT, m_Load_Text_Print);
}


BEGIN_MESSAGE_MAP(Load_Text_Dialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &Load_Text_Dialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &Load_Text_Dialog::OnBnClicked_SelectFolder)
END_MESSAGE_MAP()


// Load_Text_Dialog 메시지 처리기입니다.


BOOL Load_Text_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetDlgItemText (IDC_SCP_EDIT, _T(""));
	//*텍스트 세팅

	m_Load_Text_Progress.SetRange32 (0, 100);
	//*프로그레스 바 세팅

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void Load_Text_Dialog::OnBnClickedOk()
{
	CString Dir_Path;
	GetDlgItemText (IDC_SCP_EDIT, Dir_Path);
	if (Dir_Path.Compare(_T("")) == 0) { AfxMessageBox (_T("폴더를 지정하세요.")); return; }
	//*디렉토리명 확인

	Load_Text_Thread = AfxBeginThread (Load_Text_Func, this);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	//*텍스트 로드 스레드 가동, Load 버튼 비활성화
}


UINT Load_Text_Dialog::Load_Text_Func (LPVOID lParam) 
{
	Load_Text_Dialog *dlg = (Load_Text_Dialog*)lParam;
	//*객체 얻기

	CString Dir_Path;
	dlg->GetDlgItemText (IDC_SCP_EDIT, Dir_Path);
	//*디렉토리명 얻기

	if (Load_Tmptxt (dlg, Dir_Path) != 0) {
		
		Select_CP_Dialog sdlg;
		while (sdlg.DoModal() != IDOK) { ; }
		Load_Text_From_Directory_Data (dlg, Dir_Path, sdlg.CP_Code, true);
		//***코드페이지 지정하고 디렉토리에서 직접 로드***

	}
	//*텍스트 로드 함수 호출. tmptxt 파일을 로드했다가 실패하면 다이얼로그 창을 띄워 코드를 선택하도록 한다

	m_G_ListCtrl_Files->Invalidate();
	m_G_ListCtrl_Text->Invalidate();
	//*텍스트창 다시 그리기

	dlg->GetDlgItemText (IDC_SCP_EDIT, dlg->m_FP_tmp);
	//*폴더명 지정

	AfxMessageBox (_T("텍스트 로드 완료!"));
	dlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
	//*완료 메시지 띄우기, Load 버튼 활성화

	return 0;
}


// Load_Text_Dialog.cpp : 구현 파일입니다.
//

void Load_Text_Dialog::OnBnClicked_SelectFolder()
{

	BROWSEINFO BrInfo;
	TCHAR szBuffer[512];

	ZeroMemory (&BrInfo, sizeof(BROWSEINFO));
	ZeroMemory (szBuffer, 512);

	BrInfo.hwndOwner = m_hWnd;
	BrInfo.lpszTitle = _T("Select Folder to Load Text");
	BrInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
	BrInfo.lParam = (LPARAM)(_T("C:\\"));
	
	LPITEMIDLIST pItemIdList = SHBrowseForFolder (&BrInfo);
	SHGetPathFromIDList (pItemIdList, szBuffer);

	SetDlgItemText (IDC_SCP_EDIT, szBuffer);
	//*data로 쓰일 디렉토리 선택 후 반영

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void Load_Text_Dialog::OnCancel()
{
	DWORD ExitCode;

	if (Load_Text_Thread != NULL) {
		GetExitCodeThread (Load_Text_Thread->m_hThread, &ExitCode);
		if (ExitCode == STILL_ACTIVE) {
			
			Load_Text_Thread->SuspendThread();
			int ret = AfxMessageBox(_T("텍스트 로드를 멈추겠습니까?"), MB_YESNO);
			//*만일 파일 분해 스레드가 돌아가고 있다면 일시정지시킨 후 계속할 거냐 묻는다.

			if (ret == IDYES) {
				TerminateThread (Load_Text_Thread->m_hThread, 0);
				CloseHandle (Load_Text_Thread->m_hThread);
				All_List_Item_Delete();
				All_Text_Data_Reset();
				//*취소할 때는 리스트 데이터를 지우고 텍스트 리스트를 해제해야 한다
			}
			else {
				Load_Text_Thread->ResumeThread();
				return;
			}
			//*계속한다면 다시 재개 후 돌아가고, 그만둔다면 그대로 스레드를 종료시킨 후 OnCancel을 호출한다

		}
	}

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}
