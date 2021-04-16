
// MFCProj2Dlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CMFCProj2Dlg 대화 상자
class CMFCProj2Dlg : public CDialogEx
{
// 생성입니다.
public:
	CMFCProj2Dlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_TFW_MAIN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

public:

	CStatic m_OnList;
	CListCtrl m_ListCtrl_Files;
	CListCtrl m_ListCtrl_Text;
	CFont m_fontBold;

	afx_msg void OnBnClicked_Extract_Wolf();
	afx_msg void OnBnClicked_Make_Wolf();
	afx_msg void OnBnClicked_Change_Exe_File();

	afx_msg void OnBnClicked_Load_Text();
	afx_msg void OnBnClicked_Save_Text();
	afx_msg void OnBnClicked_Prepare_To_Translate();
	afx_msg void OnBnClicked_Auto_Trans_Papago();
//	afx_msg void OnBnClicked_Narrow_to_Em();
//	afx_msg void OnBnClicked_Check_Filename();
	afx_msg void OnBnClicked_Edit_GameDat_File();

	afx_msg void OnBnClicked_Import_List();
	afx_msg void OnBnClicked_Export_List();
	//*각 버튼에 대한 함수

	afx_msg void On_Files_List_Ctrl_NMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void On_Text_List_NMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	//*각 줄마다 리스트 색 지정 함수

	afx_msg void On_Files_List_NMClickList(NMHDR *pNMHDR, LRESULT *pResult);
	//*파일 리스트 원소를 클릭 시 동작을 지정할 함수
	
	afx_msg void On_Text_List_NMClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void On_Text_List_NMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	//*텍스트 리스트의 원소를 클릭, 더블클릭 시 동작을 지정할 함수

	void Copy_String_To_Clipboard (CString *str);
	void Get_String_From_Clipboard (CString *str);
	//*클립보드에서 문자열을 복사하는 함수, 클립보드에서 문자열을 얻어오는 함수

	void Replace_String ();
	//*리스트 문자열을 바꿔주는 함수. 파일 문자열, 텍스트 헤더 제외

	void Find_String (unsigned int Direction, CString Word);
	//*특정 문자열을 찾아주는 함수. 파일 문자열, 텍스트 헤더 제외

	void Collect_Clipboard_String_From_List ();
	void Set_List_String_From_Clipboard ();
	//*리스트에서 클릭된 문자열 복사해서 클립보드에 붙여넣기, 클립보드에서 얻어와 리스트에 붙여넣기

	void Rid_Space (CString *str);
	//*번역되어 온 텍스트의 무의미한 빈 공간을 싹 없애주는 함수

	afx_msg void OnLbnSelchangeList2();
	afx_msg void OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult);
};
