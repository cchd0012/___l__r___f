
// MFCProj2Dlg.cpp : 구현 파일
//
#include <errno.h>

#include "stdafx.h"
#include "MFCProj2.h"
#include "MFCProj2Dlg.h"
#include "afxdialogex.h"

#include "Autotrans_Papago_Dialog.h"

#include "Replace_Text_Dialog.h"
#include "File_Or_Text_Dialog.h"
#include "Find_Text_Dialog.h"

#include "Extract_Dialog.h"
#include "Pack_Dialog.h"

#include "Load_Text_Dialog.h"
#include "Save_Text_Dialog.h"
#include "Select_CP_Dialog.h"
#include "Edit_Text_Dialog.h"
#include "Check_FDName_Dialog.h"

#include "Import_List_Dialog.h"

#include "Wolf\Wolf_Functions.h"
#include "Wolf\Wolf_Text.h"
#include "Wolf\Wolf_ETC.h"
#include "Wolf\Wolf_Auto_Papago_Translate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


TCHAR *CodePageMenu[] = {_T("CP932(jap)"), _T("CP949(kor)")};
//*코드페이지 라디오 메뉴에 쓰일 문자열과 언어코드

TCHAR *Prefix_Text_Str[] = {_T(" ≫ (Jap)"), _T(" ≫ (Kor)")};
unsigned int Prefix_Length;
//*텍스트 Prefix용 문자열과 길이

TCHAR *T_Head_Start = _T("[--Text in ");
TCHAR *T_Head_End = _T("--]");
//*헤더 텍스트용 문자열

TCHAR *T_Head_Dirty = _T("(*)");
unsigned int Dirty_Prefix_Length;
//*Dirty 텍스트용 문자열과 길이

TCHAR *Copy_Text_Str[] = {_T("Jap"), _T("Kor")};
unsigned int Copy_Length;
//*Copy용 문자열과 길이

unsigned int Find_Direction;
CString Find_Str;
//*문자열을 찾을 때 한 번 찾았던 방향과 낱말을 저장하기 위한 변수

unsigned int Clicked_Idx;

CString Wolf_File_ListName;
CString Wolf_File_List;
//*리스트 이름, 리스트 내용을 기록하는 데 쓰일 CString
//*리스트가 비었으면 Export List는 비활성화를 시킨다

CString Dir_That_Has_Text_Data;
//*텍스트 데이터를 가진 폴더 이름

CListCtrl *m_G_ListCtrl_Files;
CListCtrl *m_G_ListCtrl_Text;
//*파일 이름, 텍스트 리스트

DIR_TXT_IDX m_Text_Idx_In_Dir;
//*한 디렉토리 내에 있는 모든 텍스트 데이터를 총괄하는 변수

wchar_t N_table [0x3D] = {
	0x3002, 0x300C, 0x300D, 0x3001, 0x30FB, 0x30F2, 0x30A1, 0x30A3,
	0x30A5, 0x30A7, 0x30A9, 0x30E3, 0x30E5, 0x30E7, 0x30C3, 0x30FC,
	0x30A2, 0x30A4, 0x30A6, 0x30A8, 0x30AA, 0x30AB, 0x30AD, 0x30AF,
	0x30B1, 0x30B3, 0x30B5, 0x30B7, 0x30B9, 0x30BB, 0x30BD, 0x30BF,
	0x30C1, 0x30C4, 0x30C6, 0x30C8, 0x30CA, 0x30CB, 0x30CC, 0x30CD,
	0x30CE, 0x30CF, 0x30D2, 0x30D5, 0x30D8, 0x30DB, 0x30DE, 0x30DF,
	0x30E0, 0x30E1, 0x30E2, 0x30E4, 0x30E6, 0x30E8, 0x30E9, 0x30EA,
	0x30EB, 0x30EC, 0x30ED, 0x30EF, 0x30F3
};
//*반각 일본어 테이블, 나머지 2개는 앞쪽 값을 변경시키는 데 쓰일 변수 역할이다


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCProj2Dlg 대화 상자




CMFCProj2Dlg::CMFCProj2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCProj2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCProj2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTNAME, m_OnList);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl_Files);
	DDX_Control(pDX, IDC_LIST2, m_ListCtrl_Text);
}

BEGIN_MESSAGE_MAP(CMFCProj2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCProj2Dlg::OnBnClicked_Extract_Wolf)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCProj2Dlg::OnBnClicked_Make_Wolf)
	ON_LBN_SELCHANGE(IDC_LIST2, &CMFCProj2Dlg::OnLbnSelchangeList2)
	ON_BN_CLICKED(IDC_BUTTON6, &CMFCProj2Dlg::OnBnClicked_Export_List)
	ON_BN_CLICKED(IDC_BUTTON5, &CMFCProj2Dlg::OnBnClicked_Import_List)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCProj2Dlg::OnBnClicked_Load_Text)
	ON_BN_CLICKED(IDC_BUTTON4, &CMFCProj2Dlg::OnBnClicked_Save_Text)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, &CMFCProj2Dlg::On_Files_List_Ctrl_NMCustomdraw)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST2, &CMFCProj2Dlg::On_Text_List_NMCustomdrawList)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CMFCProj2Dlg::On_Files_List_NMClickList)
	ON_NOTIFY(NM_CLICK, IDC_LIST2, &CMFCProj2Dlg::On_Text_List_NMClickList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST2, &CMFCProj2Dlg::On_Text_List_NMDblclkList)
	ON_BN_CLICKED(IDC_BUTTON7, &CMFCProj2Dlg::OnBnClicked_Prepare_To_Translate)
	ON_BN_CLICKED(IDC_BUTTON8, &CMFCProj2Dlg::OnBnClicked_Auto_Trans_Papago)
//	ON_BN_CLICKED(IDC_BUTTON8, &CMFCProj2Dlg::OnBnClicked_Narrow_to_Em)
//	ON_BN_CLICKED(IDC_BUTTON9, &CMFCProj2Dlg::OnBnClicked_Check_Filename)
	ON_BN_CLICKED(IDC_BUTTON10, &CMFCProj2Dlg::OnBnClicked_Edit_GameDat_File)
	ON_BN_CLICKED(IDC_BUTTON11, &CMFCProj2Dlg::OnBnClicked_Change_Exe_File)
END_MESSAGE_MAP()


// CMFCProj2Dlg 메시지 처리기

BOOL CMFCProj2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	CRect cr;

	Wolf_File_ListName = _T("");
	Wolf_File_List.Empty();
	SetDlgItemText(IDC_LISTNAME, _T("--No List--"));
	//*파일 참조 리스트 이름, 내용 초기화
	
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE);
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE);
	//*Make wolf file, Load text from data를 제외한 텍스트 기작, export list 버튼 비활성화.
	//*처음에는 당연히 텅 비었기 때문.

	m_ListCtrl_Files.GetWindowRect (&cr);
	m_ListCtrl_Files.SetExtendedStyle (LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ListCtrl_Files.InsertColumn (0, _T("Files"), LVCFMT_CENTER, cr.Width() - GetSystemMetrics(SM_CXFRAME)/2, -1);
	m_ListCtrl_Files.ModifyStyle (0, LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS, TRUE);
	//*파일 리스트 스타일 설정 및 초기화(헤더는 굳이 드러낼 필요 없으니 숨긴다)
	//*파일 리스트 가로 길이 : 원래 주어진 대로(정확히 맞추기)

	m_ListCtrl_Text.GetWindowRect (&cr);
	m_ListCtrl_Text.SetExtendedStyle (LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ListCtrl_Text.InsertColumn (0, _T("Text"), LVCFMT_CENTER, cr.Width() - GetSystemMetrics(SM_CXFRAME)/2, -1);
	m_ListCtrl_Text.ModifyStyle (0, LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS, TRUE);
	//*텍스트 스타일 설정 및 초기화(헤더는 굳이 드러낼 필요 없으니 숨긴다)
	//*텍스트 리스트 가로 길이 : 원래 주어진 대로(정확히 맞추기)

	memset (&m_Text_Idx_In_Dir, 0, sizeof(DIR_TXT_IDX));
	//*텍스트 총괄 변수 초기화

	CFont* pFont = GetFont();
	if (pFont){
		LOGFONT logfont;
		pFont->GetLogFont(&logfont);
		logfont.lfWeight = FW_BOLD;
		m_fontBold.CreateFontIndirect(&logfont);
	}
//	else { m_fontBold = *pFont; }
	//*굵은 폰트 생성

	Prefix_Length = (unsigned int)wcslen(Prefix_Text_Str[0]) + 8;
	Dirty_Prefix_Length = (unsigned int)wcslen(T_Head_Dirty);
	Copy_Length = (unsigned int)wcslen(Copy_Text_Str[0]);
	//Prefix_Length 설정, +8이 붙은 이유는 "[%06d]"가 들어가기 때문이다

	Load_Hanja();
	//*한자 텍스트 불러오기

	Find_Direction = NON_DIRECTION;
	Find_Str = _T("");
	Clicked_Idx = 0xFFFFFFFF;
	//*방향키 참조, 낱말 초기화

	translateInit();
	//***번역준비를 위한 초기화

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}


void CMFCProj2Dlg::OnCancel()
{

	if (m_Text_Idx_In_Dir.Is_Dirty) {
		int ret = AfxMessageBox (_T("변경했지만 저장되지 않은 텍스트가 있습니다.\n이대로 끝내겠습니까?"), MB_YESNO);
		if (ret == IDNO) { return; }
	}
	//***Dirty한 텍스트가 있다면 끌 거냐 안 끌 거냐 물어본다

	All_Text_Data_Reset();
	//*리스트 데이터 제거, 할당했던 m_Text_Idx_In_Dir 변수에 있던 값은 해제하고 끝내야 함
	//*Load_Text_Dialog.h 파일에 선언됨

	Release_Hanja();
	//*한자 텍스트 해제하기

	translateTerminate();
	//*번역 관련 요소 종결

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CMFCProj2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMFCProj2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMFCProj2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CMFCProj2Dlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if ((pMsg->message == WM_KEYDOWN) && 
		((pMsg->wParam == VK_ESCAPE) || (pMsg->wParam == VK_RETURN))) { return true; }
	//*엔터 및 ESC 눌러도 안 닫히게 할 수 있다.

	if ((pMsg->hwnd == m_ListCtrl_Text) && (m_Text_Idx_In_Dir.File_Num != 0)) {
		if (pMsg->message == WM_KEYDOWN) {

			if (GetKeyState(VK_CONTROL) && (pMsg->wParam == 0x43)) {
				Collect_Clipboard_String_From_List(); return true;
			}
			//*Ctrl + c(텍스트 복사)

			if (GetKeyState(VK_F3) && (pMsg->wParam == 0x72)) {
				Find_String (Find_Direction, Find_Str); return true;
			}
			else if (GetKeyState(VK_CONTROL) && (pMsg->wParam == 0x46)) {
				Find_String (NON_DIRECTION, _T("")); return true;
			}
			//*F3(Ctrl + f와 비슷하나 방향과 낱말이 지정되어 있다면 그대로 시행한다)
			//*Ctrl + f(특정 단어를 포함한 텍스트 찾기, 방향, 낱말 초기화)

			if (GetKeyState(VK_CONTROL) && (pMsg->wParam == 0x52)) {
				Replace_String(); return true;
			}
			//*Ctrl + r(특정 단어를 다른 단어로 교체, 파일 문자열은 제외)

			if (GetKeyState(VK_CONTROL) && (pMsg->wParam == 0x56)) {
				Set_List_String_From_Clipboard(); return true;
			}
			//*Ctrl + v(텍스트 리스트에 붙여넣기)

		}
	}
	//*(복사/붙여넣기를 적용 시) 텍스트 리스트 컨트롤에 포커스가 주어졌고 리스트가 로드됐을 때

	return CDialogEx::PreTranslateMessage(pMsg);
}








void CMFCProj2Dlg::OnBnClicked_Extract_Wolf()
{
	//***Extract Wolf File***

	Extract_Dialog dlg;
	dlg.DoModal();
	//*파일 분해 다이얼로그 호출
	//*해당 다이얼로그에서 확인 버튼을 누르면 IDOK 값이 돌아온다.

	if (!Wolf_File_List.IsEmpty()) { 
		GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE); 
		SetDlgItemText(IDC_LISTNAME, _T("List : ") + Wolf_File_ListName);
	}
	//*만일 이 과정을 거치고 리스트 변수에 리스트값이 들어왔다면 export list 버튼 활성화

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMFCProj2Dlg::OnBnClicked_Make_Wolf()
{
	//***Make Wolf File***

	if (Wolf_File_List.IsEmpty()) { 
		AfxMessageBox (_T("합성에 참고할 리스트가 없습니다."));
		return;
	}
	//*만일 리스트 변수에 리스트값이 안 들어왔다면 실행 불가하도록 한다.
	
	Pack_Dialog dlg;
	dlg.DoModal();
	//*파일 병합 다이얼로그 호출
	//***해당 다이얼로그에서 확인 버튼을 누르면 IDOK 값이 돌아온다.

	if (Wolf_File_List.IsEmpty()) { 
		GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE); 
		SetDlgItemText(IDC_LISTNAME, _T("--No List--"));
	}
	//*만일 이 과정을 거치고 리스트 변수가 비었다면 export list 버튼 비활성화

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMFCProj2Dlg::OnBnClicked_Change_Exe_File()
{
	//***Modify '.exe' File***
	
	int ret = AfxMessageBox (_T("해당 기능은 울프툴 Type 1/2에서만 정상작동합니다.\nType 3에서는 쓰지 않는 것을 권장합니다.\n계속 진행하겠습니까?"), MB_YESNO);
	//*메시지 띄우기

	if (ret == IDYES) {

		TCHAR szFilter[] = _T("Wolf Game exe File (Game.exe) |Game.exe|");
		CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilter);

		while (dlg.DoModal() != IDOK) { AfxMessageBox (_T("Game.exe 파일을 선택하세요.")); }
		//*파일 선택 다이얼로그

		if (dlg.GetPathName().GetBuffer() != NULL) {
			Exe_Change (dlg.GetPathName().GetBuffer());
			AfxMessageBox (_T("Game_New.exe 파일을 만들었습니다."));
			//*낮은 파일 버전에서 한글 표기를 할 수 있도록 바꾸어 현재 폴더에 Game_New.exe 파일을 만든다.
		}

	}

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}








void CMFCProj2Dlg::OnBnClicked_Load_Text()
{
	//***Load Text From Data***

	//***저장 안 된 텍스트가 있다면 저장하고 닫을거냐, 돌아갈 거냐 물어보고 진행한다
	//***저장할 거라 하면 Save Text To Data 버튼을 눌렀을 때처럼 먼저 한 번 수행한다

	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE);
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE);
	//*기타 기능 비활성화

	m_G_ListCtrl_Files = &m_ListCtrl_Files;
	m_G_ListCtrl_Text = &m_ListCtrl_Text;
	//*리스트 미리 지정

	Load_Text_Dialog dlg;
	dlg.DoModal();
	if (m_ListCtrl_Text.GetItemCount() != 0) { 
		GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
		GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE);
//		GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE);
		Dir_That_Has_Text_Data = dlg.m_FP_tmp;
	}
	//*텍스트 로드 다이얼로그 호출, 정상 로드되면 나머지 텍스트 버튼 활성화, 폴더경로 받아놓기

	//*문자열 일괄 변환함수(그러니까 replace 함수)도 넣고 다이얼로그로 만들자.
	//*텍스트 리스트에서 파일 제목을 클릭하면 그 안에 포함된 모든 텍스트가 클릭되도록 한다.
	//*파일 리스트에서 텍스트가 있는 파일을 클릭하면 텍스트 리스트에서 자동으로 그 파일 제목까지 이동시킨다.

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMFCProj2Dlg::OnBnClicked_Save_Text()
{
	//***Save Text To Data***

	if (m_Text_Idx_In_Dir.Is_Dirty) {

		m_G_ListCtrl_Files = &m_ListCtrl_Files;
		m_G_ListCtrl_Text = &m_ListCtrl_Text;
		//*리스트 미리 지정
		
		GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE); 
		GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE); 
		GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE); 
		GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE); 
//		GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE); 
		//*미리 버튼 비활성화

		Save_Text_Dialog dlg; dlg.m_FP_tmp = Dir_That_Has_Text_Data;
		dlg.DoModal();
		//*Dirty 상태일 때만 호출, 폴더경로 지정

		m_ListCtrl_Files.Invalidate();
		m_ListCtrl_Text.Invalidate();
		//*텍스트창 다시 그리기
		
		GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
		GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
		GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
		GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//		GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
		//*다시 버튼 활성화

	}

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMFCProj2Dlg::OnBnClicked_Prepare_To_Translate()
{
	//***Prepare to Translate***
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE); 
	//*편집 중 로드/저장/편집 불가

	Select_CP_Dialog sdlg;
	int T_List_Idx;
	CString tmpWStr, apply_Txt;

	while (sdlg.DoModal() != IDOK) { ; }
	//*코드페이지 지정

	m_Text_Idx_In_Dir.Is_Dirty = true;
	//*Dirty 상태 적용

	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {
		
		T_List_Idx = Get_List_Index (i, 0);
		if (!m_Text_Idx_In_Dir.File_Idx_Arr[i].Is_FileTxt_Dirty) {
			T_List_Idx--;
			apply_Txt = T_Head_Dirty; apply_Txt += m_ListCtrl_Text.GetItemText (T_List_Idx, 0);
			m_ListCtrl_Text.SetItemText (T_List_Idx, 0, apply_Txt);
			m_Text_Idx_In_Dir.File_Idx_Arr[i].Is_FileTxt_Dirty = true;
			T_List_Idx++;
		}
		//*파일 헤더명 Dirty 상태로 바꾸기

		for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;j++) {

			tmpWStr = m_ListCtrl_Text.GetItemText ((T_List_Idx + j), 0);
			if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_Text_Dirty) {
				tmpWStr = tmpWStr.Right (tmpWStr.GetLength() - Prefix_Length - Dirty_Prefix_Length);
			}
			else {
				tmpWStr = tmpWStr.Right (tmpWStr.GetLength() - Prefix_Length);
			}
			//*Dirty 헤더를 제외한 순수 텍스트 얻기

			m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Lang_Code = sdlg.CP_Code;
			m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_Text_Dirty = true;
			//*코드 바꾸기, Dirty 상태 적용

			apply_Txt.Format (_T("%s[%06d]%s%s"), 
				Prefix_Text_Str[sdlg.CP_Code], j, T_Head_Dirty, tmpWStr);
			m_ListCtrl_Text.SetItemText ((T_List_Idx + j), 0, apply_Txt);
			//*Dirty 헤더, 변경된 코드 헤더가 적용된 텍스트를 집어넣기

		}

	}
	//*모든 텍스트(파일 텍스트 포함)의 코드페이지를 일괄 지정, Dirty 상태 고려
	//*이 함수가 호출되면 텍스트는 이미 로드되었으므로 문제없다
	//*[set all text cp]

	Narrow_to_Em();
	//*반각 일본어 유니코드를 전각 일본어 유니코드로 바꾸기
	//*이 함수가 호출되면 텍스트는 이미 로드되었으므로 문제없다
	//*[Narrow Text to Em]

	Check_FDName_Dialog chkdlg;
	chkdlg.code = sdlg.CP_Code;
	chkdlg.DoModal();
	//*파일/디렉토리 이름 체크
	//*[Check File/Dir Name]

	m_ListCtrl_Files.Invalidate();
	m_ListCtrl_Text.Invalidate();
	//*텍스트창 다시 그리기
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
	//*편집 끝

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMFCProj2Dlg::OnBnClicked_Auto_Trans_Papago()
{
	//***Auto Trans [Papago]***
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE); 
	//*편집 중 로드/저장/편집 불가

	Autotrans_Papago_Dialog atdlg;
	atdlg.DoModal();
	m_Text_Idx_In_Dir.Is_Dirty = true;
	m_Text_Idx_In_Dir.File_Idx_Arr[atdlg.progressing_file_idx].Is_FileTxt_Dirty = true;
	//*번역 진행

	m_ListCtrl_Files.Invalidate();
	m_ListCtrl_Text.Invalidate();
	//*텍스트창 다시 그리기
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
	//*편집 끝
}


/*
void CMFCProj2Dlg::OnBnClicked_Narrow_to_Em()
{
	//***Narrow Text to Em***
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE); 
	//*편집 중 로드/저장/편집 불가

	Narrow_to_Em();
	//*반각 일본어 유니코드를 전각 일본어 유니코드로 바꾸기
	//*이 함수가 호출되면 텍스트는 이미 로드되었으므로 문제없다
	
	m_ListCtrl_Files.Invalidate();
	m_ListCtrl_Text.Invalidate();
	//*텍스트창 다시 그리기
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
	//*편집 끝

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMFCProj2Dlg::OnBnClicked_Check_Filename()
{
	//***Check File/Dir Name***
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE); 
	//*편집 중 로드/저장/편집 불가

	Select_CP_Dialog sdlg;
	while (sdlg.DoModal() != IDOK) { ; }
	//*코드페이지 선택

	Check_FDName_Dialog chkdlg;
	chkdlg.code = sdlg.CP_Code;
	chkdlg.DoModal();
	//*파일/디렉토리 이름 체크

	m_ListCtrl_Files.Invalidate();
	m_ListCtrl_Text.Invalidate();
	//*텍스트창 다시 그리기
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
	//*편집 끝

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
*/


void CMFCProj2Dlg::OnBnClicked_Edit_GameDat_File()
{
	//***Edit 'Game.dat'***

	Edit_Game_Dat();
	//*BasicData/Game.dat 파일 편집 (있을 때만)

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}








void CMFCProj2Dlg::OnBnClicked_Import_List()
{
	//***Import List***

	if (!Wolf_File_List.IsEmpty()) {
		int ret = AfxMessageBox (_T("기존에 있던 리스트가 덮어씌워집니다.\n계속 진행하겠습니까?"), MB_YESNO);
		if (ret == IDNO) { return; }
	}
	//*리스트를 계속 불러올 건가 확인

	Wolf_File_ListName = _T("");
	Wolf_File_List.Empty();
	//*리스트 초기화
	
	Import_List_Dialog dlg;
	if (dlg.DoModal() == IDOK) {
		AfxMessageBox (Wolf_File_ListName + _T(" : 리스트를 불러왔습니다."));
		//*리스트 확인 창
	}
	//*디렉토리 내부에서 리스트를 만들 것인가, 텍스트 파일을 불러올 것인가 결정하는 다이얼로그 창 띄우기

	if (Wolf_File_ListName.Compare(_T("")) != 0) {
		SetDlgItemText(IDC_LISTNAME, _T("List : ") + Wolf_File_ListName);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE); 
	}
	//*변수 갱신 및 버튼 활성화 (물론 기능이 수행됐을 때만)

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMFCProj2Dlg::OnBnClicked_Export_List()
{
	//***Export List***

	FILE *stream;
	unsigned short uni_code = 0xFEFF;

	_wfopen_s (&stream, Wolf_File_ListName.GetBuffer(), L"w+b");
	fwrite (&uni_code, sizeof(short), 1, stream);
	fwrite (Wolf_File_List.GetBuffer(), sizeof(wchar_t), Wolf_File_List.GetLength(), stream);
	fclose (stream);
	//*리스트 변수에 저장된 파일 내보내기 (이름 : (wolf 이름).txt)
	//*리스트 내용 : 유니코드 형식)

	AfxMessageBox (Wolf_File_ListName + _T(" : 리스트를 내보냈습니다."));
	//*출력 확인창

	Wolf_File_ListName = _T("");
	Wolf_File_List.Empty();
	GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE);
	SetDlgItemText(IDC_LISTNAME, _T("--No List--"));
	//*내보내고 나면 리스트 이름과 내용은 비우고 Export List 버튼을 비활성화시킨다

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}








void CMFCProj2Dlg::On_Files_List_Ctrl_NMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	if (pNMCD->dwDrawStage == CDDS_PREPAINT) {
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT) {
		if (m_Text_Idx_In_Dir.File_Num != 0) {
			if (m_Text_Idx_In_Dir.File_Idx_Arr[pLVCD->nmcd.dwItemSpec].Text_Cnt != 0) {
				if (!m_Text_Idx_In_Dir.File_Idx_Arr[pLVCD->nmcd.dwItemSpec].Is_FileTxt_Dirty) {
					pLVCD->clrText = RGB (20, 20, 75);
					pLVCD->clrTextBk = RGB (200, 200, 250);
				}
				else {
					pLVCD->clrText = RGB (20, 75, 75);
					pLVCD->clrTextBk = RGB (200, 250, 250);
				}
			}
			//*텍스트가 있는 파일에 한해서만 색상 변경하기
			//*Clean한 상태 시 텍스트 지정 = RGB (20, 20, 75), 배경색 지정 = RGB (200, 200, 250)
			//*Dirty한 상태 시 텍스트 지정 = RGB (20, 75, 75), 배경색 지정 = RGB (200, 250, 250)
		}
		*pResult = CDRF_DODEFAULT;
	}
	//*파일 리스트 내부 원소의 조건을 파악하고 색깔을 지정하기 위함이다
}


void CMFCProj2Dlg::On_Text_List_NMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	if (pNMCD->dwDrawStage == CDDS_PREPAINT) {
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT) {
		if (m_Text_Idx_In_Dir.File_Num != 0) {

			*pResult = CDRF_DODEFAULT;

			unsigned int f_idx, t_idx;
			Get_Fidx_Tidx ((unsigned int)pLVCD->nmcd.dwItemSpec, &f_idx, &t_idx);
			//*텍스트 인덱스 얻어오기

			if ((t_idx == 0) || 
				((m_Text_Idx_In_Dir.File_Idx_Arr[f_idx].Text_Idx_Arr[t_idx - 1].File_Str_Idx != LOADING_FILE) 
				&& (m_Text_Idx_In_Dir.File_Idx_Arr[f_idx].Text_Idx_Arr[t_idx - 1].File_Str_Idx != NONE_FILE))) {
				if (t_idx != 0) {
					pLVCD->clrText = RGB (20, 75, 20);
					pLVCD->clrTextBk = RGB (200, 250, 200);
				}
				//*진짜 텍스트 내용일 때
				else {
					SelectObject(pNMCD->hdc, m_fontBold.GetSafeHandle());
					*pResult = CDRF_NEWFONT;
				}
				//*텍스트 앞에 있는 헤더일 때 -> 굵은 폰트로 변경
			}
			else if (m_Text_Idx_In_Dir.File_Idx_Arr[f_idx].Text_Idx_Arr[t_idx - 1].Is_Text_Dirty) {
				pLVCD->clrText = RGB (75, 75, 20);
				pLVCD->clrTextBk = RGB (250, 250, 200);
			}
			//*텍스트 로드를 마친 경우, 텍스트 내용이 파일명일 경우에 한해서만 색상 변경하기
			//*텍스트 지정 = RGB (20, 75, 20), 배경색 지정 = RGB (200, 250, 200)
			//*t_idx에 - 1이 붙은 이유는 앞에 헤더 때문임
			//*그냥 문자열 중 Dirty한 상태 시 텍스트 지정 = RGB (75, 75, 20), 배경색 지정 = RGB (250, 250, 200)
		}
		else { *pResult = CDRF_DODEFAULT; }
	}
	//*텍스트 리스트 내부 원소의 조건을 파악하고 색깔을 지정하기 위함이다
}




void CMFCProj2Dlg::On_Files_List_NMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE); 
	//*편집 중 로드/저장/편집 불가

	int index = pNMItemActivate->iItem; 
	//*클릭한 아이템의 인덱스 얻기

	if (index >= 0 && index < m_ListCtrl_Files.GetItemCount()){

		if (m_Text_Idx_In_Dir.File_Idx_Arr[index].Text_Cnt != 0) {
			POINT point;
			m_ListCtrl_Text.GetItemPosition (m_Text_Idx_In_Dir.File_Idx_Arr[index].Idx_of_Text_Start, &point);
			CSize sz(0, point.y);
			m_ListCtrl_Text.Scroll(sz);
		}
		//*텍스트가 있는 파일 항목을 클릭하면 텍스트 리스트를 그쪽으로 이동시킨다.

	}
	//*클릭한 아이템 인덱스가 출력된 리스트 안에 있을 때 

	//*파일 리스트의 원소를 클릭했을 때 텍스트 리스트를 이동시켜 클릭 상태로 만들기 위함이다.
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
	//*편집 해제
}





void CMFCProj2Dlg::On_Text_List_NMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	int index = pNMItemActivate->iItem; 
	//*클릭한 아이템의 인덱스 얻기

	if (index >= 0 && index < m_ListCtrl_Text.GetItemCount()){
		Clicked_Idx = (unsigned int)index;
	}
	//*값만 갱신하기?
}




void CMFCProj2Dlg::On_Text_List_NMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(FALSE); 
	//*편집 중 로드/저장/편집 불가

	int index = pNMItemActivate->iItem; 
	//*더블클릭한 아이템의 인덱스 얻기

	if (index >= 0 && index < m_ListCtrl_Text.GetItemCount()){

		unsigned int fidx, tidx, FH_idx;
		CString tmptt, tmptt2, tt;

		Get_Fidx_Tidx ((unsigned int)index, &fidx, &tidx); 
		FH_idx = (unsigned int)index - tidx;
		//*파일 인덱스, 텍스트 인덱스, 텍스트 헤더 인덱스 얻기

		if (tidx == 0)
		{ 
			GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
			GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
			GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
			GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//			GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
			return; 
		}
		//*텍스트 헤더일 때는 작업하지 않는다

		else if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].File_Str_Idx != NONE_FILE)
		{

			File_Or_Text_Dialog FDd;
			if (FDd.DoModal() != IDOK) { goto DBCL_END; }
			if (FDd.Type == 2) { goto TEXT_EDIT; }
			//*파일명일 때는 체크창 하나 더 띄워서 파일 이름으로 편집할 건지,
			//*아니면 일반적인 텍스트로 편집할 건지 선택할 수 있도록 한다

			Change_FDName_Dialog CFDdlg;
			unsigned int File_Idx;
			CString FileName, FileName_Chg;
			CString FullPath, FullPath_Chg;
			CString Root_Dir;
			CString apl_txt;

			File_Idx = m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].File_Str_Idx;
			FileName = CFDdlg.Input_WStr = m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].File_Name;
			CFDdlg.Output_WStr = m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].File_Name;
			CFDdlg.code = m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Lang_Code;
			CFDdlg.Meta_type = FILE_DB;
			CFDdlg.ShouldChange = false;
			//*파일 인덱스를 얻고 그에 해당하는 파일명을 넣어 바꾸도록 한다

			if (CFDdlg.DoModal() == IDOK) {

				FileName_Chg = CFDdlg.Output_WStr;
				if (Root_Dir == _T("")) {
					for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {
						tt = m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath;
						tt.Replace (_T("\\"), _T("/"));
						if (Root_Dir.Compare(_T("")) == 0) { Root_Dir = tt; }
						else {
							while (wcsstr(tt, Root_Dir.GetBuffer()) == 0) {
								Root_Dir = Root_Dir.Left (Root_Dir.GetLength() - 1);
							}
						}
					}
				}
				//*루트 디렉토리 추려내고 순수하게 바꿀 경로 추려내기.

				FullPath = m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].File_FullPath;
				FullPath_Chg = FullPath.Left (FullPath.GetLength() - FileName.GetLength());
				FullPath_Chg += FileName_Chg;
				//*파일명, 전체경로 바꿔치기

				if (FD_Rename (FullPath.GetBuffer(), FullPath_Chg.GetBuffer()) == -1) {
					AfxMessageBox (_T("파일명을 바꾸지 못했습니다.\n파일 사용중이거나 중복된 파일명입니다."));
					GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
					GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
					GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
					GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//					GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
					return; 
				}
				//*파일명 바꾸기, 못 바꾸면 사용 중이거나 중복으로 처리

				memset (m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].File_Name, 0, (sizeof(wchar_t)*MAX_PATH));
				memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].File_Name, 
					FileName_Chg.GetBuffer(), (sizeof(wchar_t)*FileName_Chg.GetLength()));
				memset (m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].File_FullPath, 0, (sizeof(wchar_t)*MAX_PATH));
				memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].File_FullPath, 
					FullPath_Chg.GetBuffer(), (sizeof(wchar_t)*FullPath_Chg.GetLength()));
				//*해당 파일명을 포함한 File_Idx_Arr 바꾸기

				tmptt = FullPath.Right (FullPath.GetLength() - Root_Dir.GetLength());
				tmptt2 = FullPath_Chg.Right (FullPath_Chg.GetLength() - Root_Dir.GetLength());
				//*바꿀 문자열 세팅

				for (unsigned int i = 0;i < (unsigned int)m_ListCtrl_Text.GetItemCount();i++) {

					Get_Fidx_Tidx ((unsigned int)i, &fidx, &tidx); 
					if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].File_Str_Idx == File_Idx) {
						
						tt = m_ListCtrl_Text.GetItemText (i, 0);
						if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty) {
							tt = tt.Right (tt.GetLength() - Prefix_Length - Dirty_Prefix_Length);
							//* Dirty할 때는 '*'를 고려해야 한다
						}
						else {
							tt = tt.Right (tt.GetLength() - Prefix_Length);
						}
						//*순수 텍스트 구하기

						tt.Replace (tmptt, tmptt2);
						apl_txt.Format (_T("%s[%06d]%s%s"), Prefix_Text_Str[CFDdlg.code], (tidx - 1), T_Head_Dirty, tt);
						m_ListCtrl_Text.SetItemText (i, 0, apl_txt);
						//*텍스트 바꿔치고 Dirty 상태 세팅

						if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty) {
							m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty = true;
						}
						//*텍스트 Dirty 세팅
						
						if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty) {
							FH_idx = Get_List_Index (fidx, 0) - 1;
							apl_txt = T_Head_Dirty; apl_txt += m_ListCtrl_Text.GetItemText (FH_idx, 0);
							m_ListCtrl_Text.SetItemText (FH_idx, 0, apl_txt);
							m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty = true;
						}
						//*파일 더티 세팅
						
						if (!m_Text_Idx_In_Dir.Is_Dirty) { m_Text_Idx_In_Dir.Is_Dirty = true; }
						//*자체 더티 세팅

					}
				}
				//*모든 파일 텍스트들 중 해당 File_Idx를 파일 인덱스로 가진 문자열들 싹 바꾸기, Dirty 고려

			}
			//*파일명이 달라질 때만 바꾸면 됨

		}
		//*파일 헤더일 때는 파일 이름을 바꿀 수 있는 다이얼로그 창을 띄우고 적용하면 바꾸게 한다
		//*언어코드는 원래 있던 Lang_Code로 파악한다, 어차피 직접 편집은 불가능하니 통일되어 있을 것이다

		else {

TEXT_EDIT:

			Edit_Text_Dialog ETdlg;

			ETdlg.m_Input_CP = m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Lang_Code;
			ETdlg.m_Output_CP = KOR_CODE;						//*일반적으로 한글번역할 테니 이러는 게 낫다
			tmptt = m_ListCtrl_Text.GetItemText (index, 0);

			if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty) {
				tmptt = tmptt.Right (tmptt.GetLength() - Prefix_Length - Dirty_Prefix_Length);
				//* Dirty할 때는 '*'를 고려해야 한다
			}
			else {
				tmptt = tmptt.Right (tmptt.GetLength() - Prefix_Length);
			}
			//*Dirty 및 기타 헤더 없애주기

			if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_OneChar_Enter) {
				tmptt.Replace(_T("\n"), _T("\r\n"));
				tmptt.Replace(_T("\r\r"), _T("$$\r\n"));
			}
			//*0xA만으로 개행할 경우 필요 조치를 취함
			ETdlg.m_Input_tmp = ETdlg.m_Output_tmp = tmptt;
			//*다이얼로그를 구동시키기 전에 필요한 값을 넣어준다(Dirty 상태 고려)

			if (ETdlg.DoModal() == IDOK) {

				m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Lang_Code = ETdlg.m_Output_CP;

				CString apl_txt;
				apl_txt.Format (_T("%s[%06d]%s%s"), Prefix_Text_Str[ETdlg.m_Output_CP], (tidx - 1), T_Head_Dirty, ETdlg.m_Output_tmp);
				if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_OneChar_Enter) {
					apl_txt.Replace(_T("$$\r\n"), _T("\r\r"));
					apl_txt.Replace(_T("\r\n"), _T("\n"));
				}
				m_ListCtrl_Text.SetItemText (index, 0, apl_txt);
				//*0xA만으로 개행할 경우 필요 조치를 취해서 다시 반영한다

				if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty) {
					m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty = true;
				}
				if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty) {
					apl_txt = T_Head_Dirty; apl_txt += m_ListCtrl_Text.GetItemText (FH_idx, 0);
					m_ListCtrl_Text.SetItemText (FH_idx, 0, apl_txt);
					m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty = true;
				}
				if (!m_Text_Idx_In_Dir.Is_Dirty) {
					m_Text_Idx_In_Dir.Is_Dirty = true;
				}
				//*바뀐 코드페이지 적용, dirty 상태 적용, 파일 헤더 텍스트 Dirty 적용, 리스트에 바뀐 문자열 반영

				m_ListCtrl_Files.Invalidate();
				m_ListCtrl_Text.Invalidate();
				//*텍스트창 다시 그리기
			}

		}
		//*텍스트라면 텍스트 편집 창을 띄우도록 한다

	}
	//*클릭한 아이템 인덱스가 출력된 리스트 안에 있을 때 

	m_ListCtrl_Files.Invalidate();
	m_ListCtrl_Text.Invalidate();
	//*텍스트창 다시 그리기

DBCL_END:
	
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE); 
//	GetDlgItem(IDC_BUTTON9)->EnableWindow(TRUE); 
	//*편집 끝

	//*만일 파일 문자열이 아니면 텍스트 에디트 다이얼로그를 띄워 편집하고
	//*결과가 IDOK면 얻은 텍스트를 리스트에 적용한다
}




void CMFCProj2Dlg::Copy_String_To_Clipboard (CString *str)
{
    
	if (OpenClipboard() == FALSE) { AfxMessageBox (_T("클립보드를 열지 못했습니다!")); return; }
    EmptyClipboard();
	
    HGLOBAL hGlobal = GlobalAlloc (GHND | GMEM_SHARE, (str->GetLength() + 1) * sizeof(TCHAR));
    LPTSTR pGlobal = (LPTSTR)GlobalLock(hGlobal);
	if (pGlobal == NULL) { AfxMessageBox (_T("힙 주소를 얻지 못했습니다!")); return; }
	lstrcpy (pGlobal, str->GetBuffer());
    GlobalUnlock (hGlobal);
    SetClipboardData (CF_UNICODETEXT, hGlobal);
	//*윈도우 영역 힙 할당 후 그쪽으로 복사

    CloseClipboard();
	//*윈도우 영역 힙에 있는 데이터를 클립보드로 복사. 클립보드는 공용이므로 힙 할당을 해제하면 안 된다.
}
//*문자열을 클립보드로 복사해주는 함수


void CMFCProj2Dlg::Get_String_From_Clipboard (CString *str)
{
	CString ret = _T("");

	if (OpenClipboard() == FALSE) { AfxMessageBox (_T("클립보드를 열지 못했습니다!")); return; }
	HANDLE hClipboardData = GetClipboardData (CF_UNICODETEXT);
	//*클립보드에서 유니코드 텍스트 데이터 얻기

	if (hClipboardData == NULL) { AfxMessageBox (_T("텍스트를 얻어오지 못했습니다!")); return; }
	TCHAR *pchData = (TCHAR*)GlobalLock (hClipboardData);
	if (pchData == NULL) { AfxMessageBox (_T("힙 주소를 얻지 못했습니다!")); return; }
	ret = pchData;
	GlobalUnlock (hClipboardData);
    CloseClipboard();
	//*유니코드 텍스트 주소를 얻어 복사

	if (str != NULL) { (*str) = ret; }

}
//*클립보드에서 문자열을 얻어오는 함수. 복사하지 못하면 빈 문자열을 반환한다




void CMFCProj2Dlg::Replace_String ()
{
	unsigned int fidx, tidx, FH_idx;
	CString tmpWStr, cc;

	CString bef_str, aft_str;
	//*다이얼로그를 호출해서 지정한다

	if (m_ListCtrl_Text.GetItemCount() > 0) {

		Replace_Text_Dialog dlg;
		if (dlg.DoModal() != IDOK) { return; }
		bef_str = dlg.strtofind;
		aft_str = dlg.strtochg;

		for (unsigned int i = 0;i < (unsigned int)m_ListCtrl_Text.GetItemCount();i++) {

			Get_Fidx_Tidx (i, &fidx, &tidx);
			//*인덱스 얻기

			if ((tidx == 0) || 
				((!dlg.FCH) && (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].File_Str_Idx != NONE_FILE))) 
			{ continue; }
			//*헤더 문자열과 파일 문자열이 아닐 때만 바꾼다

			tmpWStr = m_ListCtrl_Text.GetItemText (i, 0);
			if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty) {
				tmpWStr = tmpWStr.Right (tmpWStr.GetLength() - Prefix_Length - Dirty_Prefix_Length);
			}
			else {
				tmpWStr = tmpWStr.Right (tmpWStr.GetLength() - Prefix_Length);
			}
			//*Dirty 헤더를 제외한 순수 텍스트 얻기

			cc = tmpWStr; cc.Replace (bef_str, aft_str);
			if (cc != tmpWStr) {

				if (dlg.code == NON_CODE) {
					tmpWStr.Format (_T("%s[%06d]%s%s"), 
						Prefix_Text_Str[m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Lang_Code], 
						(tidx - 1), T_Head_Dirty, cc.GetBuffer());
					m_ListCtrl_Text.SetItemText (i, 0, tmpWStr);
					//*리스트에 반영
				}
				else {
					tmpWStr.Format (_T("%s[%06d]%s%s"), 
						Prefix_Text_Str[dlg.code], 
						(tidx - 1), T_Head_Dirty, cc.GetBuffer());
					m_ListCtrl_Text.SetItemText (i, 0, tmpWStr);
					//*리스트에 반영
				}

				if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty) {
					m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty = true;
				}
				if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty) {
					FH_idx = Get_List_Index (fidx, 0) - 1;
					tmpWStr = T_Head_Dirty; tmpWStr += m_ListCtrl_Text.GetItemText (FH_idx, 0);
					m_ListCtrl_Text.SetItemText (FH_idx, 0, tmpWStr);
					m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty = true;
				}
				if (!m_Text_Idx_In_Dir.Is_Dirty) { m_Text_Idx_In_Dir.Is_Dirty = true; }
				//*Dirty 상태 갱신

			}
			//*바꾸고 난 문자열에 변화가 생길 때만 집어넣어주면 된다

		}
		

		m_ListCtrl_Files.Invalidate();
		m_ListCtrl_Text.Invalidate();
		//*텍스트창 다시 그리기

	}

}
//*텍스트 리스트 내 문자열을 바꿔주는 함수. 파일 문자열, 텍스트 헤더 제외




void CMFCProj2Dlg::Find_String (unsigned int _Direction, CString _Word)
{

	unsigned int Direction;
	CString Word, ListStr;
	//*방향과 낱말

	int nItem = -1;
	unsigned int T_List_Idx, fidx, tidx;
	//*텍스트의 종류(헤더 혹은 실제 텍스트)를 얻기 위한 변수
	
	POINT point;
	CSize sz;
	//*리스트를 이동시킬 때 필요한 변수

	if (m_ListCtrl_Text.GetItemCount() > 0) {

		nItem = m_ListCtrl_Text.GetNextItem (nItem, LVNI_SELECTED);
		//*클릭된 인덱스 찾기 -> 여기서 뭔가 꼬이나 봄.

		if ((_Direction == NON_DIRECTION) || (_Word.GetLength() == 0)) {
			Find_Text_Dialog dlg;
			if (dlg.DoModal() != IDOK) { Find_Direction = NON_DIRECTION; Find_Str = _T(""); return; }
			Direction = Find_Direction = dlg.Direction;
			Word = Find_Str = dlg.Find_Str;
		}
		else {
			Direction = _Direction;
			Word = _Word;
		}
		//*만일 찾는 방향이나 찾을 낱말이 지정되어 있지 않다면 다이얼로그를 호출해서 지정한다
		//*물론 Find_Direction이나 Find_Str도 지정한다
		//*지정되어 있다면 그대로 받아들이고, 찾기를 취소하면 전부 초기화한다

		T_List_Idx = (unsigned int)nItem;

		for (;((T_List_Idx > 0) && (Direction == UP_DIRECTION)) || 
			((T_List_Idx < (unsigned int)m_ListCtrl_Text.GetItemCount()) && (Direction == DOWN_DIRECTION));) {

			ListStr = m_ListCtrl_Text.GetItemText (T_List_Idx, 0);
			Get_Fidx_Tidx (T_List_Idx, &fidx, &tidx);
			if (tidx == 0) { goto MOV_LIST; }
			//*헤더면 볼 필요도 없다

			if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty) {
				ListStr = ListStr.Right (ListStr.GetLength() - Prefix_Length - Dirty_Prefix_Length);
			}
			else {
				ListStr = ListStr.Right (ListStr.GetLength() - Prefix_Length);
			}
			//*당연히 문자열은 Dirty한 거 싹 빼고 해야 한다

			if (ListStr.Find(Word) != -1) { 
				if (T_List_Idx != (unsigned int)nItem) { break; }
			}
			//*문자열 찾기, 찾으면 그대로 빠져나가기
			//*맨 처음에 문자열이 있는 텍스트를 클릭했을 수가 있는데, 그 때는 계속 움직이면 된다

MOV_LIST:
			if (Direction == UP_DIRECTION) { T_List_Idx--; }
			else { T_List_Idx++; }
			//*방향에 따라 인덱스 증감

		}

		if (((T_List_Idx == 0) && (Direction == UP_DIRECTION)) || 
			((T_List_Idx == (unsigned int)m_ListCtrl_Text.GetItemCount()) && (Direction == DOWN_DIRECTION))) {
			AfxMessageBox (_T("해당 방향으로 해당 텍스트를 가진 문자열이\n더 이상 없습니다."));
			return;
		}
		//*만일 해당 방향으로 텍스트가 없으면 해당 문자열을 포함한 텍스트가 없다고 띄우기
		
		m_ListCtrl_Text.GetItemPosition ((int)T_List_Idx, &point);
		sz.cx = 0; sz.cy = point.y;
		m_ListCtrl_Text.Scroll(sz);
		Get_Fidx_Tidx ((unsigned int)T_List_Idx, &fidx, &tidx);
		m_ListCtrl_Text.SetItemState (-1, 0, LVIS_SELECTED);
		m_ListCtrl_Text.SetItemState ((int)T_List_Idx, LVIS_SELECTED, LVIS_SELECTED);
		//*해당 텍스트만 클릭 상태로 띄우고 이동시키기

	}

}
//*텍스트 리스트 내에서 특정 문자열을 찾아주는 함수. 파일 문자열, 텍스트 헤더 제외




void CMFCProj2Dlg::Collect_Clipboard_String_From_List ()
{
	unsigned int uSelectedCount;
	unsigned int fidx, tidx;
	int nItem;
	CString totalWStr, tmpWStr, cc;
	//*리스트로 복사하는 데 필요한 변수들
	//*fidx, tidx는 Dirty한 상태인지 얻기 위해 필요하다

	if (m_ListCtrl_Text.GetItemCount() > 0) {

		uSelectedCount = m_ListCtrl_Text.GetSelectedCount(); //*선택된 갯수 얻기
		if (uSelectedCount != 0) {

			nItem = -1; totalWStr = _T("");

			for (unsigned int i = 0;i < uSelectedCount;i++) {
				nItem = m_ListCtrl_Text.GetNextItem (nItem, LVNI_SELECTED);
				Get_Fidx_Tidx (nItem, &fidx, &tidx);

				if ((tidx == 0) 
					|| (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].File_Str_Idx != NONE_FILE)) { continue; }
				//*파일 문자열과 헤더가 아닐 때만 모으기

				tmpWStr = m_ListCtrl_Text.GetItemText (nItem, 0);
				if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_Text_Dirty) {
					tmpWStr = tmpWStr.Right (tmpWStr.GetLength() - Prefix_Length - Dirty_Prefix_Length);
				}
				else {
					tmpWStr = tmpWStr.Right (tmpWStr.GetLength() - Prefix_Length);
				}
				//*Dirty 헤더를 제외한 순수 텍스트 얻기

				if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Is_OneChar_Enter) {
					tmpWStr.Replace(_T("\n"), _T("||"));
					tmpWStr.Replace(_T("\r\r"), _T("$$$"));
				}
				else { tmpWStr.Replace(_T("\r\n"), _T("||")); }
				//*0xA 하나로 개행하는 경우 또한 고려하여 개행 문자열을 Replace한다

				cc.Format(_T("[::][%s][%06d][%06d][::]%s[::]\r\n"), 
					Copy_Text_Str[m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx - 1].Lang_Code], 
					(fidx + 1), (tidx), tmpWStr.GetBuffer());
				totalWStr += cc;
				//*텍스트 모으기
			}

			Copy_String_To_Clipboard (&totalWStr);
			//*복사할 문자열을 모아 클립보드에 붙여넣기

		}
		else {
			AfxMessageBox (_T("텍스트를 선택한 후에 복사하세요."));
			//*텍스트 리스트에서 아무것도 클릭되지 않으면 클릭하라고 메시지를 띄운다
		}

	}
}
//*리스트에서 선택된 문자열 복사하여 클립보드에 붙여넣기, 파일 문자열은 제외


void CMFCProj2Dlg::Set_List_String_From_Clipboard ()
{
	CString str;
	CString tp_str, lang_str, txt_str, list_str;
	unsigned int Total_strNum = 0, fidx, tidx, Start_Pos, End_Pos;
	unsigned int T_List_Idx;
	unsigned int code;

	Get_String_From_Clipboard (&str);
	//*클립보드에서 문자열 얻기

	str.Replace (_T("] ["), _T("]["));
	str.Replace (_T(" ::"), _T("::"));
	str.Replace (_T(":: "), _T("::"));
	str.Replace (_T("[: :]"), _T("[::]"));
	str.Replace (_T("[::] "), _T("[::]"));
	str.Replace (_T(" [::]"), _T("[::]"));
	str.Replace (_T(" : "), _T(":"));
	str.Replace (_T("@ "), _T("@"));
	str.Replace (_T("[:]"), _T("[::]"));
	Rid_Space (&str);
	//*의미없는 빈 공간 전부 없애기, 파파고에도 적용되도록 수정

	for (unsigned int i = 0;i < (unsigned int)str.GetLength() - 1;i++) {
		if ((str[i] == 0xD) && (str[i+1] == 0xA)) { Total_strNum++; }
	}
	if (!((str[str.GetLength() - 2] == 0xD) && (str[str.GetLength() - 1] == 0xA))) 
	{ Total_strNum++; }
	//*총 수 : 개행 수 + 1(번역하면서 없어지는듯)

	Start_Pos = End_Pos = 0; 
	for (unsigned int i = 0;i < Total_strNum;i++) {

		End_Pos = Start_Pos;
		while ((str.Mid(End_Pos, 6) != _T("[::]\r\n")) && (End_Pos < (unsigned int)str.GetLength())) 
		{ End_Pos++; }
		tp_str = str.Mid (Start_Pos, (End_Pos - Start_Pos));
		if (tp_str.Right(4) == _T("[::]")) { tp_str = tp_str.Left(tp_str.GetLength() - 4); }
		//*각 문자열의 시작 위치, 끝 위치 얻고 문자열 얻기

		lang_str = tp_str.Mid (5, 3);
		fidx = _ttoi(tp_str.Mid (10, 6));
		tidx = _ttoi(tp_str.Mid (18, 6));
		txt_str = tp_str.Right (tp_str.GetLength() - 29);
		//*문자열을 하나하나 옮겨주기

		if ((fidx == 0) || (tidx == 0)) {
			AfxMessageBox (_T("error : 올바르지 않은 문자열을 붙여넣었습니다."));
			return;
		}
		fidx--; tidx--;
		//***만일 fidx나 tidx 중 0이 나왔다면 잘못된 문자열을 찾아 복사한 것이니 에러로 처리한다

		if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx].Is_OneChar_Enter) { 
			txt_str.Replace (_T("||"), _T("\n")); 
			txt_str.Replace (_T("$$$"), _T("\r\r")); 
		}
		else  { txt_str.Replace (_T("||"), _T("\r\n")); }
		//*0xD, 0xA로 개행인가, 0xA만 가지고 개행이냐에 따라 적절히 교체해주기
		
		if (lang_str == Copy_Text_Str[KOR_CODE]) { code = KOR_CODE; }
		else if (lang_str == Copy_Text_Str[JAP_CODE]) { code = JAP_CODE; }
		//*코드 맞추기

		T_List_Idx = Get_List_Index (fidx, tidx);
		list_str.Format (_T("%s[%06d]%s%s"), Prefix_Text_Str[code], tidx, T_Head_Dirty, txt_str);
		m_ListCtrl_Text.SetItemText (T_List_Idx, 0, list_str);
		//*해당 리스트 위치에 등록
		
		if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx].Is_Text_Dirty) {
			m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx].Is_Text_Dirty = true;
		}
		if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty) {
			m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty = true;
		}
		if (!m_Text_Idx_In_Dir.Is_Dirty) { m_Text_Idx_In_Dir.Is_Dirty = true; }
		//*Dirty 상태 갱신
		
		Start_Pos = End_Pos + 6;
		//*다음 시작을 위해 갱신

	}

	m_ListCtrl_Files.Invalidate();
	m_ListCtrl_Text.Invalidate();
	//*텍스트창 다시 그리기

}
//*클립보드에서 문자열을 얻어와 리스트에 반영하기, 파일 문자열은 제외




void CMFCProj2Dlg::Rid_Space (CString *str)
{
	bool NoTrivialSpace = false;
	unsigned int i, Mid_start, Mid_end;
	CString Midstr, bfr_chg, aft_chg;
	
	while (!NoTrivialSpace) {

		i = 0;
		while (i < ((unsigned int)str->GetLength() - 3)) {

			if ((str->GetBuffer()[i] == '\\') && (str->GetBuffer()[i+1] == ' ')){

				Mid_start = i + 2; i += 2;
				//*%s가 시작되는 위치 잡고 i 갱신

				while ((i < ((unsigned int)str->GetLength() - 1)) && 
					!((str->GetBuffer()[i] == ' ') && (str->GetBuffer()[i+1] == '['))) { i++; }
				if (i == (str->GetLength() - 1)) { NoTrivialSpace = true; break; }
				Mid_end = i; i += 2;
				//*%s가 끝나는 위치 잡고 i 갱신. 하지만 끝내는 위치를 잡지 못했을 경우 그냥 나간다
				//*어차피 첫 번째, 두 번째 while 루프에서는 자동적으로 나간다

				Midstr = str->Mid (Mid_start, (Mid_end - Mid_start));
				bfr_chg = str->Mid (Mid_start - 2, (Mid_end - Mid_start + 4));
				aft_chg = _T("\\"); aft_chg += Midstr; aft_chg += _T("[");
				str->Replace (bfr_chg, aft_chg);
				break;
				//*%s에 해당하는 문자열 얻기. %s의 시작 위치와 끝나는 위치가 같지 않으면
				//*길이가 있다는 뜻이므로 문자열을 바꾼다

			}
			else { 
				i++; 
				if (i == (str->GetLength() - 3)) { NoTrivialSpace = true; }
			}
			//'\ '가 있는 위치 잡기, 없으면 그냥 넘긴다
		}

	}

	//*정확히는 '\ ' + %s + ' ['의 형태를 지닌 문자열이 있다면 '\' + %s + '['로 바꿔주는 것.
	//*%s에 들어갈 문자열은 길이가 0이든 말든 상관 없음
}
//*번역되어 온 텍스트의 무의미한 빈 공간을 싹 없애주는 함수




void CMFCProj2Dlg::OnLbnSelchangeList2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMFCProj2Dlg::OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 이 기능을 사용하려면 Windows XP 이상이 필요합니다.
	// _WIN32_WINNT 기호는 0x0501보다 크거나 같아야 합니다.
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}