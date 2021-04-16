#pragma once
#include "afxwin.h"
#include "afxcmn.h"

void All_List_Item_Delete();
void All_Text_Data_Reset();
//*리스트 데이터를 없애주는 함수, DIR_TXT_IDX 구조체 초기화 함수

void Get_Fidx_Tidx (unsigned int Idx, unsigned int *fidx, unsigned int *tidx);
//*들어온 인자값(텍스트 리스트 인덱스)을 바탕으로 파일 인덱스와 텍스트 인덱스를 저장시켜주는 함수
//*사실 tidx는 1 빼야 한다

int Get_List_Index (unsigned int fidx, unsigned int tidx);
//*들어온 인자값(파일 인덱스, 텍스트 인덱스)을 바탕으로 리스트 인덱스를 반환하는 함수
//*조건에 안 맞는다 싶으면 -1 반환


// Load_Text_Dialog 대화 상자입니다.

class Load_Text_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Load_Text_Dialog)

public:
	Load_Text_Dialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~Load_Text_Dialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_LOAD_TEXT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClicked_SelectFolder();

	CEdit m_FolderPath;
	CString m_FP_tmp;
	
	CWinThread *Load_Text_Thread;
	static UINT Load_Text_Func (LPVOID lParam);
	CProgressCtrl m_Load_Text_Progress;
	CStatic m_Load_Text_Print;
};