#include "stdafx.h"

#include "Wolf_Text.h"


extern CListCtrl *m_G_ListCtrl_Files;
extern CListCtrl *m_G_ListCtrl_Text;
extern DIR_TXT_IDX m_Text_Idx_In_Dir;
//*extern으로 리스트 변수를 얻어옴 

extern TCHAR *Prefix_Text_Str[];
extern unsigned int Prefix_Length;
//*Prefix용 문자열

extern TCHAR *T_Head_Start;
extern TCHAR *T_Head_End;
//*텍스트 헤더용 문자열

extern TCHAR *T_Head_Dirty;
extern unsigned int Dirty_Prefix_Length;
//*Dirty Prefix 문자열

extern wchar_t N_table[];
//*반각->전각에서 참고할 테이블

unsigned int Total_Cnt_File_Num;
unsigned int Cnt_File_Num;
unsigned int Cnt_Text_Num;
//*파일 인덱스 및 텍스트 인덱스를 전역적으로 카운트한다

wchar_t *JapHanja;
wchar_t *KorHanja;
unsigned int HanjaCnt;
//*일본한자 <-> 한국한자 변환용 버퍼, 그리고 크기


//*Game.dat은 여기서 적용하지 않는다.
//*다른 함수를 만들어 거기서 중점적으로 수정하도록 함.


inline void Lower (CString *str) 
{
	wchar_t *tt = str->GetBuffer();
	for (int i = 0;i < str->GetLength();i++) { 
		if ((tt[i] >= 'A') && (tt[i] <= 'Z')) { tt[i] += ('a' - 'A'); } 
		else if (tt[i] == '\\') { tt[i] = '/'; }
	}
}
//*문자열 소문자로 맞추기, '\'는 '/'로 일괄 통일한다


void Load_Text_From_Directory_Data (Load_Text_Dialog *LoadTtxDlg, CString DirPath, unsigned int code, bool IsRoot)
{
	WIN32_FIND_DATAW FData;
	HANDLE hFind;
	//*디렉토리 접근용 핸들

	HANDLE hEachFile;
	unsigned char *EachFileBuff;
	unsigned int EachFileSize;
	DWORD lp_read;
	//*각 파일 접근용 핸들, 변수

	CString FileName_Str;
	CString Cnt_File_Num_Str;
	CString File_FullPath;
	//*각자 이름을 담을 문자열과 전체경로를 담을 문자열이 된다
	
	CString Text_Str;
	wchar_t *tmpWTStr;
	//*임시 텍스트 문자열



	if (IsRoot) { 

		LoadTtxDlg->SetDlgItemText (IDC_LT_TEXT, _T("Loading Progress : Load Text From Files..."));

		Cnt_File_Num = Cnt_Text_Num = 0; 
		//*인덱스로 쓸 카운터 초기화

		Total_Cnt_File_Num = Get_Total_FileCount (DirPath, _T("Game.dat"));
		//*총 파일 수 구하기. //*여기서도 Game.dat은 제외한다

		m_Text_Idx_In_Dir.File_Num = Total_Cnt_File_Num;
		m_Text_Idx_In_Dir.File_Idx_Arr = (FILE_TXT_ELEM*)malloc (sizeof(FILE_TXT_ELEM) * Total_Cnt_File_Num);
		memset (m_Text_Idx_In_Dir.File_Idx_Arr, 0, sizeof(FILE_TXT_ELEM) * Total_Cnt_File_Num);
		//*m_Text_Idx_In_Dir 할당 및 초기화

	}
	//*맨 처음 호출했을 때 리스트 비우고 카운터 초기화.
	//*텍스트 로드 함수 (전체 파일에 대하여. 텍스트가 있는 파일은 색깔 표시)
	//*우선적으로 [디렉토리명].tmptxt를 로드한다. 다만 해당 디렉토리가 없으면 로드할 수 없고,
	//*디렉토리가 있어도 tmptxt 파일 안에 기재된 정보와 맞지 않으면 그냥 디렉토리 정보를 기반으로 로드한다(경고메시지).
	//*일단 디렉토리를 지정하고, 그에 대응하는 [디렉토리명].tmptxt 파일이 있으면 이걸 우선적으로 로드한다.



	CString DirPath_ast = DirPath + _T("/*");
	hFind = FindFirstFileW (DirPath_ast.GetBuffer(), &FData); 
	if (hFind == INVALID_HANDLE_VALUE) {
		AfxMessageBox (DirPath + _T(" : 해당 폴더/파일이 없습니다."));
		return;
	}
	//*핸들 체크



	do {

		File_FullPath = DirPath + _T("/") + FData.cFileName;
		//*전체경로 얻기

		if (lstrcmp(FData.cFileName, L"Game.dat") == 0){;}
		//*Game.dat 생략

		else if(!(FData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			&& (lstrcmp(FData.cFileName, L".") != 0) 
			&& (lstrcmp(FData.cFileName, L"..") != 0))
		{
			FileName_Str = FData.cFileName;
			//*파일 문자열 얻기

			Cnt_File_Num_Str.Format(_T("%d"), Cnt_File_Num);
			m_G_ListCtrl_Files->InsertItem (Cnt_File_Num, Cnt_File_Num_Str + _T(" : ") + FileName_Str);
			//***문자열 리스트에 넣기***

			hEachFile = CreateFileW (File_FullPath.GetBuffer(), GENERIC_READ, FILE_SHARE_READ, 
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			EachFileSize = GetFileSize (hEachFile, NULL);
			EachFileBuff = (unsigned char*)malloc (sizeof(char) * EachFileSize);
			if (!ReadFile (hEachFile, EachFileBuff, EachFileSize, &lp_read, NULL)){
				AfxMessageBox (File_FullPath + _T(" : 파일을 읽지 못했습니다."));
				LoadTtxDlg->SetDlgItemText (IDC_LT_TEXT, _T("Loading Progress : Load Failed..."));
				All_List_Item_Delete(); All_Text_Data_Reset(); return;
				//*리스트 내용 삭제 및 DIR_TXT_IDX 변수 초기화
			}
			CloseHandle (hEachFile);
			//*파일 열기, 버퍼 할당, 읽기 후 파일 닫기

			memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].File_Name, FileName_Str.GetBuffer(), sizeof(TCHAR) * FileName_Str.GetLength());
			memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].File_FullPath, File_FullPath.GetBuffer(), sizeof(TCHAR) * File_FullPath.GetLength());
			//*파일 이름, 파일 전체경로 지정, 각 텍스트당 미리 언어코드 초기화.


			if ((wcsstr (FData.cFileName, L".common") != NULL)
				|| (wcsstr (FData.cFileName, L".dat") != NULL)
				|| (wcsstr (FData.cFileName, L".dbtype") != NULL)
				|| (wcsstr (FData.cFileName, L".mps") != NULL)
				|| (wcsstr (FData.cFileName, L".project") != NULL)
				|| (wcsstr (FData.cFileName, L".tile") != NULL)) {

				m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Cnt = Get_Text_Count_Or_Set_Text (EachFileBuff, EachFileSize, code, NULL);
				//*텍스트 수만 먼저 구하기
				if (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Cnt != 0){
					m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Idx_Arr = 
						(TEXT_IDX*)malloc (sizeof(TEXT_IDX) * m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Cnt);
					memset (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Idx_Arr, 0, 
						sizeof(TEXT_IDX) * m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Cnt);
					memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].File_Name, FileName_Str.GetBuffer(), sizeof(TCHAR) * FileName_Str.GetLength());
					memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].File_FullPath, File_FullPath.GetBuffer(), sizeof(TCHAR) * File_FullPath.GetLength());
					for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Cnt;i++) 
					{ m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Idx_Arr[i].Lang_Code = code; }
					Get_Text_Count_Or_Set_Text (EachFileBuff, EachFileSize, code, &m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num]);
					//*텍스트가 0이 아니라면 텍스트 배열을 할당하고 텍스트 세팅, 코드만 미리 세팅해두고 들어간다
				}
				//*텍스트 수를 기반으로 m_Text_Idx_In_Dir 변수에 데이터 세팅
			}
			else { m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Cnt = 0; }
			//*텍스트가 있을 법한 파일들만 골라서 체크한다. 나머지는 다 텍스트 없는 걸로 취급


			if (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Cnt != 0) {
				Text_Str = T_Head_Start; Text_Str += FileName_Str; Text_Str += T_Head_End;
				m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Idx_of_Text_Start = Cnt_Text_Num;
				m_G_ListCtrl_Text->InsertItem (Cnt_Text_Num++, Text_Str);
			}
			//***헤더 텍스트 등록(텍스트 수가 0이 아닐 때만), 추가로 인덱스를 등록한다***

			for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Cnt;i++) {

				if (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Idx_Arr[i].Lang_Code == JAP_CODE) {
					tmpWTStr = J2U ((char*)(EachFileBuff + m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Idx_Arr[i].Offset + sizeof(int)));
				}
				else if (m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Idx_Arr[i].Lang_Code == KOR_CODE) {
					tmpWTStr = K2U ((char*)(EachFileBuff + m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Idx_Arr[i].Offset + sizeof(int)));
				}
				//*문자열 변환

				Text_Str.Format(_T("%s[%06d]"), Prefix_Text_Str[m_Text_Idx_In_Dir.File_Idx_Arr[Cnt_File_Num].Text_Idx_Arr[i].Lang_Code], i);
				Text_Str += tmpWTStr;
				//*문자열 확정

				m_G_ListCtrl_Text->InsertItem (Cnt_Text_Num++, Text_Str);
				free (tmpWTStr);
			}
			//***텍스트 리스트에 그대로 등록(언어코드를 따진다.)***
			//***그냥 code 변수로 하지 않고 [디렉토리명].tmptxt 파일과의 호환성을 고려해 저렇게 설정한다)***

			free (EachFileBuff);
			//*파일 버퍼 해제

			Cnt_File_Num++;
			LoadTtxDlg->m_Load_Text_Progress.SetPos (PROGRESSVALUE(Cnt_File_Num, Total_Cnt_File_Num*2));
			//*텍스트 로드 후 프로그레스 바 갱신
			//***[디렉토리명].tmptxt 파일이 있으면 File_Str_Idx 값도 갱신할 수 있으므로 그냥 1배로 설정하면 된다***

		}
		//*디렉토리가 아니라면(Game.dat 제외) 확인해보고 로드한다

		else if((lstrcmp(FData.cFileName, L".") != 0) && (lstrcmp(FData.cFileName, L"..") != 0)) {
			Load_Text_From_Directory_Data (LoadTtxDlg, File_FullPath, code, false);
		}
		//*만일 디렉토리라면(현재, 이전 디렉토리 제외) 재귀함수 호출
		//***[디렉토리].tmptxt 파일이 없을 때만 재귀호출한다

	} while (FindNextFileW (hFind, &FData));
	//*디렉토리 내의 모든 파일 다 돌아보기
	//***[디렉토리].tmptxt 파일이 없을 때만 재귀호출한다. 있으면 이 루프 한 번만 돌아도 됨.

	FindClose (hFind);
	//*핸들 닫아주기



	if (IsRoot) {

		LoadTtxDlg->SetDlgItemText (IDC_LT_TEXT, _T("Loading Progress : Check Filenames..."));

		CString fstr, tstr;

		for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {
			if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt != 0) {
				for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;j++) {

					tstr = m_G_ListCtrl_Text->GetItemText (j + m_Text_Idx_In_Dir.File_Idx_Arr[i].Idx_of_Text_Start + 1, 0); Lower (&tstr);
					//*대소문자 구분을 없애고 비교한다

					m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].File_Str_Idx = NONE_FILE;
					for (unsigned int k = 0;k < m_Text_Idx_In_Dir.File_Num;k++) {

						fstr = m_Text_Idx_In_Dir.File_Idx_Arr[k].File_FullPath; Lower (&fstr);
						fstr = fstr.Right (fstr.GetLength() - DirPath.GetLength() - 1);
						//*대소문자 구분을 없애고 비교한다(+1은 슬래시 때문임)
						//*순수 파일 문자열이 아닌 경우도 있으니 그냥 상대경로만 따와서 비교하는 게 낫다

						if (wcsstr(&tstr.GetBuffer()[Prefix_Length], fstr.GetBuffer()) != 0) {
							m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].File_Str_Idx = k; 
							break;
						}
						//*Prefix된 문자열을 제거하고 비교한다
					}

					if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].File_Str_Idx == NONE_FILE) {
						unsigned int T_List_Idx = Get_List_Index (i, j);
						Text_Str = m_G_ListCtrl_Text->GetItemText (T_List_Idx, 0);
						TCHAR *pt = Text_Str.GetBuffer();
						for (int j = 0;j < Text_Str.GetLength();j++) {
							if (pt[j] == 0x7E26) { pt[j] = 0x7E31; }
							if (pt[j] == 0x6A2A) { pt[j] = 0x6A6B; }
						}
						m_G_ListCtrl_Text->SetItemText (T_List_Idx, 0, Text_Str);
						//*"종횡"문자는 솔직히 귀찮기도 하고, 번역에도 많이 쓰일 것 같지 않으니
						//*그냥 문자열이라면 미리 바꿔두는 게 나을 것 같다
					}

					LoadTtxDlg->m_Load_Text_Progress.SetPos (50 + 
						PROGRESSVALUE(j + m_Text_Idx_In_Dir.File_Idx_Arr[i].Idx_of_Text_Start + 2, m_G_ListCtrl_Text->GetItemCount()*2));

				}
			}
		}
		//*텍스트 내용이 파일명/디렉토리명일 경우 (파일 리스트 원소 중 전체경로 문자열 맨 끝에서 텍스트 내용을 찾은 경우)
		//*파일 플래그를 세팅해준다 (디렉토리명은 맨 뒤에 '/'가 붙는다)
		//***일단 디렉토리 쪽은 나중에 문제가 생기면 손대기로 하자***

		LoadTtxDlg->SetDlgItemText (IDC_LT_TEXT, _T("Loading Progress : Saving Tmptxt File..."));

		Save_Tmptxt (DirPath);
		//*tmptxt 파일이 없을 때 진행됐을 테니 그대로 저장

		LoadTtxDlg->SetDlgItemText (IDC_LT_TEXT, _T("Loading Progress : Text Loading Finished."));

	}

}


int Save_Text_To_Directory_Data (Save_Text_Dialog *SaveTxtDlg, CString DirPath)
{

	if (!m_Text_Idx_In_Dir.Is_Dirty) { return 0; }
	//*변경된 텍스트가 있을 때만 저장, 에러는 아니니까 0을 반환한다

	HANDLE hEachFile;
	unsigned char *org_buff, *chg_buff, *org_pnt, *chg_pnt;
	unsigned int *org_ofs;
	unsigned int org_size, chg_size;
	//*파일기록에 필요한 변수들

	int T_List_Idx, Text_Gap;
	//*텍스트 리스트 원소를 얻는 데 필요한 변수, 원래 텍스트와 변형된 텍스트 길이 차이

	CString tmpCStr, tmpNStr;
	char *tmpStr;
	unsigned int code, offset;
	//*있으면 편하다
	
	bool check = false;
	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {

		if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Is_FileTxt_Dirty) {

			tmpCStr.Format(_T("Saving Text to File...(%d/%d)"), i, m_Text_Idx_In_Dir.File_Num);
			SaveTxtDlg->SetDlgItemText (IDC_ST_TEXT, tmpCStr);
			//*텍스트 출력

			hEachFile = CreateFileW (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, 
				GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hEachFile == INVALID_HANDLE_VALUE) {
				tmpNStr = m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath;
				JPHan_2_KRHan (&tmpNStr);
				memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, tmpNStr.GetBuffer(), sizeof(TCHAR)*tmpNStr.GetLength());
				tmpNStr = m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name;
				JPHan_2_KRHan (&tmpNStr);
				memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name, tmpNStr.GetBuffer(), sizeof(TCHAR)*tmpNStr.GetLength());
				hEachFile = CreateFileW (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, 
					GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hEachFile == INVALID_HANDLE_VALUE) {
					tmpCStr.Format (_T("%s : 해당 파일이 없습니다."), m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath);
					AfxMessageBox (tmpCStr);
					return -1;
				}
				//*이유는 모르겠지만 여기서 뭔가 안 바뀌는 버그나는듯
			}
			org_size = chg_size = GetFileSize (hEachFile, NULL);
			org_buff = (unsigned char*)malloc (sizeof(char) * org_size);
			ReadFile (hEachFile, org_buff, org_size, NULL, NULL);
			CloseHandle (hEachFile);
			//*새 파일버퍼, 파일버퍼 변수 초기화, 만일 파일을 못 열면 -1 반환

			T_List_Idx = Get_List_Index (i, 0);
			//*해당 파일의 첫 문자열이 있는 인덱스
			for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;j++) {

				if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_Text_Dirty) {

					tmpCStr = m_G_ListCtrl_Text->GetItemText((T_List_Idx+j), 0);
					tmpCStr = tmpCStr.Right (tmpCStr.GetLength() - Prefix_Length - Dirty_Prefix_Length);
					code = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Lang_Code;
					if (code == KOR_CODE) { tmpStr = U2K (tmpCStr.GetBuffer()); }
					else if (code == JAP_CODE) { tmpStr = U2J (tmpCStr.GetBuffer()); }
					//*코드에 따라 ansi 문자열 얻기, prefix, Dirty 헤더 문자열 고려

					Text_Gap = ((int)strlen(tmpStr)+1) - (int)m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Length;
					if (tmpStr != NULL) { free (tmpStr); }
					//*원래 문자열과 바뀐 문자열 길이차 구한 후 버퍼 해제

					if (Text_Gap != 0) { chg_size = (unsigned int)((int)chg_size + Text_Gap); }
					//*새 버퍼 크기에 텍스트 길이차 반영

				}

			}
			//*기존 값에서 텍스트를 바꾸고 할당될 크기 미리 계산(물론 Dirty한 텍스트만 계산하면 된다)
			//*파일 문자열은 편집을 할 수 없으니 당연히 Dirty한 상태가 되지 않는다

			chg_buff = (unsigned char*)malloc (sizeof(char) * chg_size);
			memset (chg_buff, 0, (sizeof(char) * chg_size));
			//*바꾼 사이즈를 가지고 할당

			org_pnt = org_buff; chg_pnt = chg_buff;
			//*포인터 초기화

			org_ofs = (unsigned int*)malloc (sizeof(int) * m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt);
			for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;j++) {
				org_ofs[j] = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Offset;
			}
			//*오프셋 원본값을 미리 보존해놓는다

			T_List_Idx = Get_List_Index (i, 0);
			//*해당 파일의 첫 문자열이 있는 인덱스
			for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;j++) {

				if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_Text_Dirty) {

					tmpCStr = m_G_ListCtrl_Text->GetItemText((T_List_Idx+j), 0);
					tmpCStr = tmpCStr.Right (tmpCStr.GetLength() - Prefix_Length - Dirty_Prefix_Length);
					code = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Lang_Code;
					if (code == KOR_CODE) { tmpStr = U2K (tmpCStr.GetBuffer()); }
					else if (code == JAP_CODE) { tmpStr = U2J (tmpCStr.GetBuffer()); }
					//*코드에 따라 ansi 문자열 얻기
					
					offset = org_ofs[j];
					memcpy (chg_pnt, org_pnt, ((org_buff + offset) - org_pnt));
					//*이전까지 복사하지 못한 길이만큼 새 버퍼에 복사
					
					chg_pnt += ((org_buff + offset) - org_pnt);
					org_pnt = (org_buff + offset);
					//*오프셋, 원래 버퍼, 새 버퍼 포인터 갱신

					*(unsigned int*)chg_pnt = ((unsigned int)strlen(tmpStr) + 1); chg_pnt += sizeof(int);
					memcpy (chg_pnt, tmpStr, (unsigned int)strlen(tmpStr)); chg_pnt += ((unsigned int)strlen(tmpStr) + 1);
					org_pnt += sizeof(int);
					org_pnt += m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Length;
					//*새 버퍼에 바뀐 텍스트 이식, 버퍼 포인터 갱신
					
					tmpNStr.Format (_T("%s[%06d]%s"), Prefix_Text_Str[code], j, tmpCStr.GetBuffer());
					m_G_ListCtrl_Text->SetItemText ((T_List_Idx+j), 0, tmpNStr);
					//*텍스트 리스트에 바뀐 텍스트 이식(Dirty 헤더 문자열 제거)

					m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_Text_Dirty = false;
					Text_Gap = ((int)strlen(tmpStr)+1) - (int)m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Length;
					m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Length = (unsigned int)(strlen(tmpStr)+1);
					if (tmpStr != NULL) { free (tmpStr); }
					//*텍스트 더티 상태 없애기, 길이 변경, 텍스트 해제

					if (Text_Gap != 0) { 
						for (unsigned int k = j+1;k < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;k++) {
							m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[k].Offset = 
								(unsigned int)((int)m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[k].Offset + Text_Gap);
						}
					}
					//*변경된 길이와 기존 오프셋의 길이 차이(Text_Gap)를 이용해 이후에 있는 오프셋 싹 갱신

				}

			}

			if (org_pnt < (org_buff + org_size)) { 
				memcpy (chg_pnt, org_pnt, ((org_buff + org_size) - org_pnt)); 
			}
			//*마지막 남은 찌꺼기 버퍼 옮기기
			
			_wremove (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath);
			hEachFile = CreateFileW (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, 
				GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			WriteFile (hEachFile, chg_buff, chg_size, NULL, NULL);
			CloseHandle (hEachFile);
			//*기존 파일 지우고 수정된 파일버퍼로 저장

			free (chg_buff); free (org_buff); free (org_ofs);
			//*원래 파일버퍼, 수정된 파일버퍼 해제, 오프셋 버퍼 해제, 파일핸들 해제

			int FH_List_Idx = Get_List_Index (i, 0) - 1;
			tmpCStr = m_G_ListCtrl_Text->GetItemText(FH_List_Idx, 0);
			tmpCStr = tmpCStr.Right (tmpCStr.GetLength() - Dirty_Prefix_Length);
			m_G_ListCtrl_Text->SetItemText (FH_List_Idx, 0, tmpCStr);
			//*파일 텍스트 헤더에서 더티 헤더 삭제

			m_Text_Idx_In_Dir.File_Idx_Arr[i].Is_FileTxt_Dirty = false;
			//*파일 더티 상태 없애기

		}
		//*변경된 텍스트가 있는 파일만 손댐

		SaveTxtDlg->m_Save_Text_Progress.SetPos (PROGRESSVALUE(i, m_Text_Idx_In_Dir.File_Num));
		//*프로그레스 바 갱신

	}

	m_Text_Idx_In_Dir.Is_Dirty = false;
	//*Dirty 상태 해제

	Save_Tmptxt (DirPath);
	//*[디렉토리명].tmptxt에 현재 상태 저장

	SaveTxtDlg->m_Save_Text_Progress.SetPos (100);
	//*프로그레스 바 갱신

	return 0;
}


	
//*파일 문자열 깨진 거 점검하는 함수, 불러온 파일 중에 그대로 저장하면 중복되는 파일 문자열이 있나 점검하는 함수,
//*그에 따라 파일명을 바꿔주는 함수도 있어야 함. 이 모든 정보는 tmptxt에 갱신되어 저장된다.
//*파일명을 바꿔주는 함수에서는 텍스트 리스트 내부의 파일명도 싸그리 바꾼다.
//*파일 문자열은 여기서 다루지 않을 테니 이건 다른 데서 만들자






int Load_Tmptxt (Load_Text_Dialog *LoadTtxDlg, CString DirPath)
{
	HANDLE hTmpTxt;
	unsigned char *TmpTxtBuff, *TmpTxtOrg, *TmpTxtPnt;
	unsigned int TmpTxtSize, TmpOrgSize;
	//*tmptxt 파일을 위한 변수들

	HANDLE hEachFile;
	unsigned char *EachFileBuff;
	unsigned int EachFileSize;
	//*각 파일을 위한 변수들

	wchar_t *tpstr;
	CString Text_In_File, Text_In_TmpTxt;
	//*비교용 문자열



	LoadTtxDlg->SetDlgItemText (IDC_LT_TEXT, _T("Loading Progress : Load Text From Files..."));

	CString TmpTxtFile = DirPath + _T(".tmptxt");
	hTmpTxt = CreateFileW (TmpTxtFile.GetBuffer(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hTmpTxt == INVALID_HANDLE_VALUE) { return -1; }
	//*파일 찾기

	TmpTxtSize = GetFileSize (hTmpTxt, NULL);
	TmpTxtBuff = (unsigned char*)malloc (sizeof(char) * TmpTxtSize);
	ReadFile (hTmpTxt, TmpTxtBuff, TmpTxtSize, NULL, NULL);
	CloseHandle (hTmpTxt);
	//*파일 로드 후 핸들 닫기

	TmpOrgSize = *(unsigned int*)TmpTxtBuff;
	TmpTxtOrg = (unsigned char*)malloc (sizeof(char) * TmpOrgSize);
	memset (TmpTxtOrg, 0, (sizeof(char) * TmpOrgSize));
	memcpy (TmpTxtOrg, TmpTxtBuff, TmpTxtSize);
	free (TmpTxtBuff);
	//*언팩할 버퍼로 복사하고 해제함

	Decode (TmpTxtOrg, TmpOrgSize);
	TmpTxtPnt = TmpTxtOrg;
	//*얻어온 원래 길이로 파일 언팩
	
	unsigned int File_Num;
	//*DIR_TXT_IDX의 변수

	TCHAR *File_Name;
	TCHAR *File_FullPath;
	unsigned int Idx_of_Text_Start;
	unsigned int Text_Cnt;
	//*FILE_TXT_ELEM의 변수들
	
	unsigned int Lang_Code;
	unsigned int Offset;
	unsigned int Length;
	unsigned int File_Str_Idx;
	unsigned int Is_OneChar_Enter;
	TCHAR *Text, *Text_bak;
	//*TEXT_IDX의 변수들과 유니코드 텍스트 포인터

	CString Text_Str;
	unsigned int F_List_Cnt_Idx, T_List_Cnt_Idx, Total_Text_Num;
	//*리스트에 추가할 문자열
	//*파일 리스트 인덱스 번호, 텍스트 리스트 인덱스 번호, 전체 텍스트 수

	char *atpstr;
	//*있으면 편한 변수



	All_List_Item_Delete();
	All_Text_Data_Reset();
	F_List_Cnt_Idx = T_List_Cnt_Idx = 0;
	//*리스트 및 DIR_TXT_IDX 변수 초기화

	File_Num = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);
	Total_Text_Num = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);

	m_Text_Idx_In_Dir.File_Num = File_Num;
	m_Text_Idx_In_Dir.File_Idx_Arr = (FILE_TXT_ELEM*)malloc (sizeof(FILE_TXT_ELEM) * File_Num);
	memset (m_Text_Idx_In_Dir.File_Idx_Arr, 0, (sizeof(FILE_TXT_ELEM) * File_Num));
	//*파일 수 확인 및 할당/초기화
	
	bool loadTmpTxt = false;
	for (unsigned int i = 0;i < File_Num;i++) {

		if (TmpTxtPnt > (TmpTxtOrg + TmpOrgSize)) { 
			AfxMessageBox (_T("1 : tmptxt 파일이 올바르지 않습니다.\n폴더에서 직접 불러옵니다."));
			free (TmpTxtOrg); All_List_Item_Delete(); All_Text_Data_Reset(); return -1;
		}

		File_Name = (TCHAR*)TmpTxtPnt; TmpTxtPnt += (sizeof(TCHAR)*MAX_PATH);
		File_FullPath = (TCHAR*)TmpTxtPnt; TmpTxtPnt += (sizeof(TCHAR)*MAX_PATH);
		Idx_of_Text_Start = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);
		Text_Cnt = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);
		//*변수 가져오기

		memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name, File_Name, (sizeof(TCHAR)*MAX_PATH));
		memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, File_FullPath, (sizeof(TCHAR)*MAX_PATH));
		m_Text_Idx_In_Dir.File_Idx_Arr[i].Idx_of_Text_Start = Idx_of_Text_Start;
		m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt = Text_Cnt;
		m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr = (TEXT_IDX*)malloc (sizeof(TEXT_IDX) * Text_Cnt);
		memset (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr, 0, (sizeof(TEXT_IDX) * Text_Cnt));
		//*변수 할당/초기화

		TCHAR *t;
		if (((t = _tcsstr (File_FullPath, L"Data/")) != NULL) || ((t = _tcsstr (File_FullPath, L"Data\\")) != NULL)) {
			TCHAR newFullPath [MAX_PATH]; memset (newFullPath, 0, sizeof(TCHAR)*MAX_PATH);
			GetFullPathName (t, MAX_PATH, newFullPath, NULL);
			File_FullPath = newFullPath;
			memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, File_FullPath, (sizeof(TCHAR)*MAX_PATH));
		}
		//*경로가 바뀌었으면 File_FullPath를 바꿔친다

		hEachFile = CreateFileW (File_FullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hEachFile == INVALID_HANDLE_VALUE) {
			AfxMessageBox (File_FullPath);
			AfxMessageBox (_T("2 : tmptxt 파일이 올바르지 않습니다.\n폴더에서 직접 불러옵니다."));
			free (TmpTxtOrg); All_List_Item_Delete(); All_Text_Data_Reset(); return -1;
		}
		EachFileSize = GetFileSize (hEachFile, NULL);
		EachFileBuff = (unsigned char*)malloc (sizeof(char) * EachFileSize);
		ReadFile (hEachFile, EachFileBuff, EachFileSize, NULL, NULL);
		CloseHandle (hEachFile);
		//*실제 파일 불러오고 핸들 닫기, 못 부르면 깨진 걸로 처리

		Text_Str.Format (_T("%d : "), F_List_Cnt_Idx); Text_Str += File_Name;
		m_G_ListCtrl_Files->InsertItem (F_List_Cnt_Idx++, Text_Str);
		//***문자열을 만들고 파일 리스트에 추가하기

		if (Text_Cnt != 0){
			Text_Str = T_Head_Start; Text_Str += File_Name; Text_Str += T_Head_End;
			m_G_ListCtrl_Text->InsertItem (T_List_Cnt_Idx++, Text_Str);
			//***문자열을 만들고 텍스트 리스트에 추가하기(이건 텍스트 수가 있을 때)
		}

		for (unsigned int j = 0;j < Text_Cnt;j++) {

			if (TmpTxtPnt > (TmpTxtOrg + TmpOrgSize)) { 
				AfxMessageBox (_T("3 : tmptxt 파일이 올바르지 않습니다.\n폴더에서 직접 불러옵니다."));
				free (TmpTxtOrg); All_List_Item_Delete(); All_Text_Data_Reset(); 
				free (EachFileBuff); return -1;
			}

			Lang_Code = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);
			Offset = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);
			Length = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);
			File_Str_Idx = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);
			Is_OneChar_Enter = *(unsigned int*)TmpTxtPnt; TmpTxtPnt += sizeof(int);
			Text_bak = Text = (TCHAR*)TmpTxtPnt; TmpTxtPnt += (sizeof(TCHAR) * (wcslen(Text)+1));
			//*변수 가져오기, 텍스트는 (유니코드 문자열+NULL)로 되어 있다 가정

			Text_Str.Format(_T("%s[%06d]"), Prefix_Text_Str[Lang_Code], j);
			if (Lang_Code == KOR_CODE) { 
				atpstr = U2K (Text); Text = K2U (atpstr); free (atpstr);
				tpstr = K2U ((char*)(EachFileBuff + Offset + sizeof(int))); 
			}
			else if (Lang_Code == JAP_CODE) { 
				atpstr = U2J (Text); Text = J2U (atpstr); free (atpstr);
				tpstr = J2U ((char*)(EachFileBuff + Offset + sizeof(int))); 
			}
			Text_In_TmpTxt = Text; free (Text);
			Text_In_File = tpstr; free (tpstr);
			if (Text_In_TmpTxt.CompareNoCase (Text_In_File) != 0) {
				if (!loadTmpTxt) {
					AfxMessageBox (_T("4 : 원본과 tmptxt 파일 간의 텍스트가 다릅니다.\ntmptxt 파일의 텍스트를 불러옵니다."));
					Text_In_File = Text_In_TmpTxt;
					loadTmpTxt = true;
				}
				else { Text_In_TmpTxt = Text_In_File; }
				//*** 여기서 한/일 한자가 달라서 에러나는 경우가 있는듯
				//*** 정확히는 tmptxt에는 수정 후 저장되고 원본이 수정되지 않은 경우
				//*** 그렇다면 원본텍스트를 수정하여 비교하는 셈 친다
				//*** 그래도 로드해야겠다면 tmptxt 텍스트를 불러온다다
			}
			//*문자열 비교(대소문자는 비교하지 않는다)
			//***이중변환하는 이유는 파일에 저장할 때 혹여나 깨졌을 문자열까지 재현하기 위함이다
			//***이외에 추가로 비교할 게 있으면 (파일 문자열 여부 같은거) 또 추가하면 된다
			
			m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Lang_Code = Lang_Code;
			m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Offset = Offset;
			m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Length = Length;
			m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].File_Str_Idx = File_Str_Idx;
			m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_OneChar_Enter = Is_OneChar_Enter;
			//*정확하다 싶으면 변수 할당/초기화

			Text_In_TmpTxt = Text_bak;
			Text_Str += Text_In_TmpTxt;
			m_G_ListCtrl_Text->InsertItem (T_List_Cnt_Idx++, Text_Str);
			//***문자열을 만들고 텍스트 리스트에 추가하기(이때는 원래 안 깨진 문자열로 추가)

			LoadTtxDlg->m_Load_Text_Progress.SetPos (PROGRESSVALUE(T_List_Cnt_Idx, Total_Text_Num));
			//*프로그레스 바 세팅

		}

		free (EachFileBuff);
		//*실제 파일 해제하기

	}
	//*이중 루프를 돌려 DIR_TXT_IDX 구조체에 체크 및 세팅

	free (TmpTxtOrg);

	LoadTtxDlg->SetDlgItemText (IDC_LT_TEXT, _T("Loading Progress : Text Loading Finished."));

	return 0;
}
//*tmptxt 파일을 로드해 DIR_TXT_IDX에 집어넣기, 리스트에 출력까지 겸함


//*tmptxt 파일에 저장된 순서 : 
//*총 파일의 수, 총 텍스트 수, 총 파일의 수만큼 FILE_TXT_ELEM 구조체 (TEXT_IDX의 크기 * 텍스트 수의 크기를 지닌 Text_Idx_Arr 포함)
//*FILE_TXT_ELEM 구조체나 TEXT_IDX는 Dirty 변수를 제외한다


void Save_Tmptxt (CString DirPath)
{
	HANDLE hTmpTxt;
	unsigned char *TmpTxtBuff, *TmpTxtComp, *TmpTxtPnt;
	unsigned int TmpTxtSize = 0, TmpcompSize;
	//*tmptxt 파일을 위한 변수들

	unsigned int List_Idx, List_Accum;
	CString List_Text;
	//*리스트 위치를 계산하고 텍스트를 얻기 위해 필요한 변수


	TmpTxtSize += sizeof(int);											//*File_Num 추가
	TmpTxtSize += sizeof(int);											//*총 텍스트 수 추가
	
	List_Accum = 0;
	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {

		TmpTxtSize += (sizeof(TCHAR)*MAX_PATH);							//*File_Name 추가
		TmpTxtSize += (sizeof(TCHAR)*MAX_PATH);							//*File_FullPath 추가
		TmpTxtSize += sizeof(int);										//*Idx_of_Text_Start 추가
		TmpTxtSize += sizeof(int);										//*Text_Cnt 추가
		//***이게 0이냐 아니냐에 따라 비교할 필요가 있을 수 있음***

		for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;j++) {

			TmpTxtSize += sizeof(int);									//*Lang_Code 추가
			TmpTxtSize += sizeof(int);									//*Offset 추가
			TmpTxtSize += sizeof(int);									//*Length 추가
			TmpTxtSize += sizeof(int);									//*File_Str_Idx 추가
			TmpTxtSize += sizeof(int);									//*Is_OneChar_Enter 추가

			List_Idx = List_Accum + 1 + j;								//*리스트 위치값 계산
			List_Text = m_G_ListCtrl_Text->GetItemText (List_Idx, 0);	
			List_Text = List_Text.Right ((unsigned int)List_Text.GetLength() - Prefix_Length);
			//*텍스트 얻어오기, Prefix된 문자열은 제외한다

			TmpTxtSize += (sizeof(TCHAR) * ((unsigned int)List_Text.GetLength()+1));	//*텍스트 추가

		} 

		if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt != 0) 
		{ List_Accum += (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt + 1); }
		//*누적값에 추가

	}
	//*기록할 크기 계산하기


	TmpTxtBuff = (unsigned char*)malloc (sizeof(char) * TmpTxtSize);
	memset (TmpTxtBuff, 0, TmpTxtSize); TmpTxtPnt = TmpTxtBuff;
	//*버퍼 할당 및 초기화

	*(unsigned int*)TmpTxtPnt = m_Text_Idx_In_Dir.File_Num; 
	TmpTxtPnt += sizeof(int);
	//*File_Num 추가

	unsigned int textnum = m_G_ListCtrl_Text->GetItemCount(); 
	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) 
	{ if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt != 0) { textnum--; } }
	*(unsigned int*)TmpTxtPnt = textnum; 
	TmpTxtPnt += sizeof(int);
	//*총 텍스트 수 추가(리스트 전체 원소에서 텍스트 수가 0이 아닌 헤더값들만 빼면 됨)
	
	List_Accum = 0;
	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {

		memcpy (TmpTxtPnt, m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name, (sizeof(TCHAR)*MAX_PATH));
		TmpTxtPnt += (sizeof(TCHAR)*MAX_PATH);
		//*File_Name 추가
		memcpy (TmpTxtPnt, m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, (sizeof(TCHAR)*MAX_PATH));
		TmpTxtPnt += (sizeof(TCHAR)*MAX_PATH);
		//*File_FullPath 추가
		*(unsigned int*)TmpTxtPnt = m_Text_Idx_In_Dir.File_Idx_Arr[i].Idx_of_Text_Start; 
		TmpTxtPnt += sizeof(int);
		//*Idx_of_Text_Start 추가
		*(unsigned int*)TmpTxtPnt = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt; 
		TmpTxtPnt += sizeof(int);
		//*Text_Cnt 추가
		
		for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;j++) {
			*(unsigned int*)TmpTxtPnt = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Lang_Code; 
			TmpTxtPnt += sizeof(int);
			//*Lang_Code 추가
			*(unsigned int*)TmpTxtPnt = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Offset; 
			TmpTxtPnt += sizeof(int);
			//*Offset 추가
			*(unsigned int*)TmpTxtPnt = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Length; 
			TmpTxtPnt += sizeof(int);
			//*Length 추가
			*(unsigned int*)TmpTxtPnt = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].File_Str_Idx; 
			TmpTxtPnt += sizeof(int);
			//*File_Str_Idx 추가
			*(unsigned int*)TmpTxtPnt = m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_OneChar_Enter; 
			TmpTxtPnt += sizeof(int);
			//*Is_OneChar_Enter 추가

			List_Idx = List_Accum + 1 + j;								//*리스트 위치값 계산
			List_Text = m_G_ListCtrl_Text->GetItemText (List_Idx, 0);
			List_Text = List_Text.Right ((unsigned int)List_Text.GetLength() - Prefix_Length);
			//*텍스트 얻어오기, Prefix된 문자열은 제외한다

			memcpy (TmpTxtPnt, List_Text.GetBuffer(), (sizeof(TCHAR) * List_Text.GetLength()));
			TmpTxtPnt += (sizeof(TCHAR) * ((unsigned int)List_Text.GetLength()+1));
			//*텍스트 추가
		}

		if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt != 0) 
		{ List_Accum += (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt + 1); }
		//*누적값에 추가
	}
	//*순차적으로 기록


	Encode (TmpTxtBuff, TmpTxtSize);
	TmpTxtComp = TmpTxtBuff;
	TmpcompSize = (*(unsigned int*)(&TmpTxtBuff[4]));
	//*버퍼 압축
	
	CString TmpTxtFile = DirPath + _T(".tmptxt");
	hTmpTxt = CreateFileW (TmpTxtFile.GetBuffer(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile (hTmpTxt, TmpTxtComp, TmpcompSize, NULL, NULL);
	free (TmpTxtBuff); CloseHandle (hTmpTxt);
	//*파일 기록

	//***Save_Text_To_Directory_Data 함수에서 호출할 때는 먼저 텍스트를 파일에 저장하고 이 함수를 호출한다
	//***그래야 오프셋과 길이를 정확히 계산할 수 있고 문자열도 정확히 얻어올 수 있다
}








void Narrow_to_Em()
{
	
	int T_List_Idx;
	bool Is_Narrow;
	CString tmpWStr, _tmp, apply_Txt;
	wchar_t tm_str[2];


	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {

		T_List_Idx = Get_List_Index (i, 0);
		for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Cnt;j++) {

			if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].File_Str_Idx != NONE_FILE) { continue; }
			//*파일 문자열이 아닐 때만 건드리기로 한다

			tmpWStr = m_G_ListCtrl_Text->GetItemText ((T_List_Idx + j), 0);
			if (m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_Text_Dirty) {
				tmpWStr = tmpWStr.Right (tmpWStr.GetLength() - Prefix_Length - Dirty_Prefix_Length);
			}
			else {
				tmpWStr = tmpWStr.Right (tmpWStr.GetLength() - Prefix_Length);
			}
			//*Dirty 헤더를 제외한 순수 텍스트 얻기

			Is_Narrow = false;
			for (unsigned int k = 0;k < (unsigned int)tmpWStr.GetLength();k++) {
				if ((tmpWStr[k] >= 0xFF61) && (tmpWStr[k] <= 0xFF9F)) { Is_Narrow = true; break; }
			}
			//*반각 문자가 있는지 체크, 반각 문자가 있을 때만 반영하면 된다

			if (Is_Narrow) {

				_tmp = _T("");
				for (unsigned int k = 0;k < (unsigned int)tmpWStr.GetLength();k++) {

					tm_str[0] = tm_str[1] = 0;

					if (BETWEEN(tmpWStr[k],0xFF61,0xFF9F)) {
						if (tmpWStr[k] == 0xFF9E) {
							if (_tmp.GetLength() == 0) { tm_str[0] = 0x3F; }
							else if (BETWEEN((_tmp.GetBuffer()[_tmp.GetLength()-1]),0x30AB,0x30C8) 
								|| BETWEEN((_tmp.GetBuffer()[_tmp.GetLength()-1]),0x30CF,0x30DB)) {
								if (BETWEEN(tmpWStr[k-1],0xFF61,0xFF9F)) { _tmp.GetBuffer()[_tmp.GetLength()-1] += 1; }
								else { tm_str[0] = 0x3F; }
							}
							else if (_tmp.GetBuffer()[_tmp.GetLength()-1] == 0x30A6) { _tmp.GetBuffer()[_tmp.GetLength()-1] = 0x30F4; }
							else { tm_str[0] = 0x3F; }
							//*1번째 변경, 만일 이전 문자가 반각이 아니었으면 물음표로 바꾸어 넣는다
						}
						else if (tmpWStr[k] == 0xFF9F) {
							if (_tmp.GetLength() == 0) { tm_str[0] = 0x3F; }
							else if (BETWEEN((_tmp.GetBuffer()[_tmp.GetLength()-1]),0x30CF,0x30DB)) {
								if (BETWEEN(tmpWStr[k-1],0xFF61,0xFF9F)) { _tmp.GetBuffer()[_tmp.GetLength()-1] += 2; }
								else { tm_str[0] = 0x3F; }
							}
							else { tm_str[0] = 0x3F; }
							//*2번째 변경, 만일 이전 문자가 반각이 아니었으면 물음표로 바꾸어 넣는다
						}
						else {
							tm_str[0] = N_table[tmpWStr[k]-0xFF61];
							//*일반 문자
						}
						//*수정하여 추가, 탁점 같은 거 있으면 앞쪽을 수정하면 된다
					}
					else { tm_str[0] = tmpWStr[k]; } //*그냥 추가
					//*반각 -> 전각, 전각은 그대로 추가

					_tmp += tm_str;
					//*문자열 더하기
				}
				//*반각 문자를 전각으로 바꾼 새 문자열 만들기

				m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Is_Text_Dirty = true;
				//*코드 바꾸기, Dirty 상태 적용
				apply_Txt.Format (_T("%s[%06d]%s%s"), 
					Prefix_Text_Str[m_Text_Idx_In_Dir.File_Idx_Arr[i].Text_Idx_Arr[j].Lang_Code], 
					j, T_Head_Dirty, _tmp);
				m_G_ListCtrl_Text->SetItemText ((T_List_Idx + j), 0, apply_Txt);
				//*Dirty 헤더, 변경된 코드 헤더가 적용된 텍스트를 집어넣기

				if (!m_Text_Idx_In_Dir.File_Idx_Arr[i].Is_FileTxt_Dirty) {
					apply_Txt = T_Head_Dirty; apply_Txt += m_G_ListCtrl_Text->GetItemText ((T_List_Idx - 1), 0);
					m_G_ListCtrl_Text->SetItemText ((T_List_Idx - 1), 0, apply_Txt);
					m_Text_Idx_In_Dir.File_Idx_Arr[i].Is_FileTxt_Dirty = true;
				}
				//*반각 문자가 있다면 파일 헤더명 Dirty 상태로 바꾸기
				if (!m_Text_Idx_In_Dir.Is_Dirty) {
					m_Text_Idx_In_Dir.Is_Dirty = true;
				}
				//*파일, 원본에도 Dirty 상태 적용
			}
			//*반각 텍스트가 있으면 당연히 전부 Dirty를 적용한다

		}

	}
	//*반각 일본어 유니코드를 전각 일본어 유니코드로 바꾸기

}








void Check_Filename_For_Codepage (Check_FDName_Dialog *ChkDlg, unsigned int code)
{

	CString Root_Dir;
	CString WStr_Name, WStr_Name_chg, WStr_FullPath, WStr_FullPath_bak;
	CString chg_Name;
	CString cc, cc_t;
	TCHAR **Dir_List;
	unsigned int Dir_Num;
	//*디렉토리 목록을 얻어올 때 필요한 변수

	char *tmpCStr; 
	wchar_t *tmpWStr;
	//*디렉토리 문자열 비교에 필요한 변수

	unsigned int fidx, tidx;
	CString List_str;
	//*리스트 반영/획득 변수

	Root_Dir = _T("");
	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {
		if (Root_Dir.Compare(_T("")) == 0) { Root_Dir = m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath; }
		else {
			while (wcsstr(m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, Root_Dir.GetBuffer()) == 0) {
				Root_Dir = Root_Dir.Left (Root_Dir.GetLength() - 1);
			}
		}
	}
	//*파일 전체의 File_FullPath에서 루트 디렉토리 추려내기(공통으로 일치하는 부분만 찾아내면 된다)

	Dir_Num = Get_Dir_Num_Or_DirList (Root_Dir.Left(Root_Dir.GetLength()-1), NULL);
	Dir_List = (TCHAR**)malloc (sizeof(TCHAR*) * Dir_Num);
	for (unsigned int i = 0;i < Dir_Num;i++) { 
		Dir_List[i] = (TCHAR*)malloc(sizeof(TCHAR) * MAX_PATH); 
		memset (Dir_List[i], 0, (sizeof(TCHAR) * MAX_PATH));
	}
	Get_Dir_Num_Or_DirList (Root_Dir.Left(Root_Dir.GetLength()-1), Dir_List);
	//*디렉토리 리스트 초기화, 루트 디렉토리 밑에 있는 디렉토리 리스트 얻기
	//*슬래시가 앞에 붙어있으니 슬래시를 빼고 넣어야 정상적으로 문자열을 얻을 수 있다

	unsigned int Total_Task = Dir_Num + m_Text_Idx_In_Dir.File_Num;
	
	for (unsigned int i = 0;i < Dir_Num;i++) {

		WStr_Name = Dir_List[i];
		WStr_Name = WStr_Name.Right (WStr_Name.GetLength() - Root_Dir.GetLength());

		cc.Format (_T("Check Folder Name : %s"), WStr_Name);
		ChkDlg->SetDlgItemText(IDC_CHK_FD_TEXT, cc);
		ChkDlg->m_Check_FDName_Progress.SetPos (PROGRESSVALUE(i, Total_Task));
		//*텍스트 지정 및 프로그레스 바 세팅
		
		cc = WStr_Name;
		if (code == KOR_CODE) { JPHan_2_KRHan (&cc); }
		//***미리 약간 손을 써 두기로 한다

		if (code == JAP_CODE) { tmpCStr = U2J(cc.GetBuffer()); tmpWStr = J2U(tmpCStr); }
		else if (code == KOR_CODE) { tmpCStr = U2K(cc.GetBuffer()); tmpWStr = K2U(tmpCStr); }
		WStr_Name_chg = tmpWStr; free (tmpCStr); free (tmpWStr);
		//*문자열 변환으로 언어에 호환 가능한지 체크

		if (WStr_Name != WStr_Name_chg) {

			Change_FDName_Dialog dlg;

			if (cc == WStr_Name_chg) { goto FOLDER_LABEL_2; }
			//*이때는 다이얼로그 호출 없이 그대로 넘어가도 된다

			WStr_FullPath.Format (_T("%s : \n코드페이지에서 폴더명을 인식할 수 없습니다.\n깨지는 문자열을 바꾸세요."), 
				WStr_Name);
			AfxMessageBox (WStr_FullPath);
			//*바뀐 문자열과 원래 문자열이 다르면 이름을 바꾸라고 메시지박스를 하나 띄운다

			dlg.Input_WStr = WStr_Name;
			dlg.Output_WStr = WStr_Name_chg;
			dlg.code = code;
			dlg.Meta_type = DIR;
			dlg.ShouldChange = true;
			//*다이얼로그 호출 전 준비

FOLDER_LABEL:
			if (dlg.DoModal() == IDOK) {

				WStr_Name_chg = dlg.Output_WStr;

FOLDER_LABEL_2:
				chg_Name = Root_Dir + WStr_Name_chg;
				//*디렉토리 이름 바꿀 준비
				
				for (unsigned int j = 0;j < Dir_Num;j++) {
					if ((j != i) && (dlg.Output_WStr.Compare(Dir_List[j]) == 0)) {
						AfxMessageBox (_T("폴더명이 중복됩니다.\n다시 입력하세요."));
						goto FOLDER_LABEL;
					}
				}
				//***현재 존재하는 폴더명들과 중복 체크를 해야 한다

				Dir_List[i][wcslen(Dir_List[i]) - 1] = 0;
				chg_Name = chg_Name.Left (chg_Name.GetLength() - 1);

				if (FD_Rename (Dir_List[i], chg_Name.GetBuffer()) == -1) {
					AfxMessageBox (_T("폴더 계층 수가 변동되거나\n폴더 내 파일을 사용중입니다."));
					for (unsigned int i = 0;i < Dir_Num;i++) { free (Dir_List[i]); } free (Dir_List);
					return;
					//*경로를 바꾸지 못하면 그냥 리턴한다
					//*이때는 잠시 마지막 슬래시값을 빼놓고 하자
				}
				else { 

					Dir_List[i][wcslen(Dir_List[i])] = '/';
					chg_Name += _T("/");
					
					cc = Dir_List[i];
					memset (Dir_List[i], 0, (sizeof(TCHAR) * MAX_PATH));
					memcpy (Dir_List[i], chg_Name.GetBuffer(), (sizeof(TCHAR) * chg_Name.GetLength()));
					for (unsigned int j = 0;j < Dir_Num;j++) {
						WStr_FullPath_bak = WStr_FullPath = Dir_List[j];
						WStr_FullPath.Replace (cc, chg_Name);
						if (WStr_FullPath_bak != WStr_FullPath) {
							memset (Dir_List[j], 0, (sizeof(TCHAR) * MAX_PATH));
							memcpy (Dir_List[j], WStr_FullPath.GetBuffer(), 
								(sizeof(TCHAR) * WStr_FullPath.GetLength()));
						}
					}
					//*Dir_List의 값도 바꿔주기

					for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Num;j++) {

						WStr_FullPath = m_Text_Idx_In_Dir.File_Idx_Arr[j].File_FullPath;
						WStr_FullPath = WStr_FullPath.Right (WStr_FullPath.GetLength() - Root_Dir.GetLength());
						WStr_FullPath_bak = WStr_FullPath;
						WStr_FullPath.Replace (WStr_Name, WStr_Name_chg);
						//*WStr_FullPath는 Data 기준 파일 상대경로를 의미한다

						if (WStr_FullPath_bak != WStr_FullPath) {

							memset (&m_Text_Idx_In_Dir.File_Idx_Arr[j].File_FullPath[Root_Dir.GetLength()], 
								0, (sizeof(TCHAR) * (MAX_PATH - Root_Dir.GetLength())));
							memcpy (&m_Text_Idx_In_Dir.File_Idx_Arr[j].File_FullPath[Root_Dir.GetLength()], 
								WStr_FullPath.GetBuffer(), (sizeof(TCHAR) * WStr_FullPath.GetLength()));
							//*File_FullPath에 새로 기록

							for (unsigned int k = 0;k < (unsigned int)m_G_ListCtrl_Text->GetItemCount();k++) {
								Get_Fidx_Tidx (k, &fidx, &tidx);
								if ((tidx != 0) && (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].File_Str_Idx == j)) {

									List_str = m_G_ListCtrl_Text->GetItemText (k, 0);
									List_str.Replace (WStr_FullPath_bak, WStr_FullPath);
									if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].Is_Text_Dirty) {
										m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].Is_Text_Dirty = true;
										List_str.Insert(Prefix_Length, T_Head_Dirty);
									}
									m_G_ListCtrl_Text->SetItemText (k, 0, List_str);
									//*텍스트 바꿔주기, Dirty 상태 고려

									if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty) {
										m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty = true;
										List_str = m_G_ListCtrl_Text->GetItemText (Get_List_Index(fidx,0)-1, 0);
										List_str.Insert (0, T_Head_Dirty);
										m_G_ListCtrl_Text->SetItemText (Get_List_Index(fidx,0)-1, 0, List_str);
									}
									//*파일 헤더 바꿔주기, Dirty 상태 고려

									if (!m_Text_Idx_In_Dir.Is_Dirty) { m_Text_Idx_In_Dir.Is_Dirty = true; }
									//*전체 Dirty 설정

								}
							}
							//*그리고 텍스트 중 해당 파일을 인덱스로 삼고 있다면 그 또한 바꿔준다
						}

					}
					//*파일 내 모든 File_FullPath 문자열을 대상으로 검사하여, 
					//*전체 경로를 바꿀 필요가 있다면 바꾸고 다시 기록해준다
					//*또한 텍스트 리스트 파일 문자열 중에서 해당 파일을 인덱스로 가졌다면 그쪽 경로도 바꿔야 한다

				}
				//*경로 변경 성공시
			}

			//*바뀐 문자열을 얻고, 문자열을 부분 경로로 가진 파일의 File_FullPath,
			//*모든 텍스트에서 원래 문자열을 포함한 파일 텍스트를 싸그리 바꿔버린다

		}
		//*언어변환을 적용한 문자열과 원래 문자열 비교

	}
	//***디렉토리 체크(File_FullPath 바꾸기)


	for (unsigned int i = 0;i < (unsigned int)m_G_ListCtrl_Text->GetItemCount();i++) {

		Get_Fidx_Tidx (i, &fidx, &tidx);
					
		if ((tidx > 0) && (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].File_Str_Idx != NONE_FILE)
			&& (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].Lang_Code != code)) {

			cc = m_G_ListCtrl_Text->GetItemText (i, 0);
			if (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].Is_Text_Dirty) {
				cc = cc.Right (cc.GetLength() - Prefix_Length - Dirty_Prefix_Length);
			}
			else {
				cc = cc.Right (cc.GetLength() - Prefix_Length);
			}
			//*Dirty 헤더를 제외한 순수 텍스트 얻기

			m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].Lang_Code = code;
			m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].Is_Text_Dirty = true;
			//*코드 바꾸기, Dirty 상태 적용

			cc_t.Format (_T("%s[%06d]%s%s"), Prefix_Text_Str[code], tidx-1, T_Head_Dirty, cc.GetBuffer());
			m_G_ListCtrl_Text->SetItemText (i, 0, cc_t);
			//*Dirty 헤더, 변경된 코드 헤더가 적용된 텍스트를 집어넣기

			if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty) {
				m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty = true;
				List_str = m_G_ListCtrl_Text->GetItemText (Get_List_Index(fidx,0)-1, 0);
				List_str.Insert (0, T_Head_Dirty);
				m_G_ListCtrl_Text->SetItemText (Get_List_Index(fidx,0)-1, 0, List_str);
			}
			//*파일 헤더 바꿔주기, Dirty 상태 고려

			if (!m_Text_Idx_In_Dir.Is_Dirty) { m_Text_Idx_In_Dir.Is_Dirty = true; }
			//*전체 Dirty 설정
		}

	}
	//***코드와 일치하지 않을 경우 먼저 다 바꿔주기
	

	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {

		cc.Format (_T("Check File Name : %s"), m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name);
		ChkDlg->SetDlgItemText(IDC_CHK_FD_TEXT, cc);
		ChkDlg->m_Check_FDName_Progress.SetPos (PROGRESSVALUE((i+Dir_Num), Total_Task));
		//*텍스트 지정 및 프로그레스 바 세팅

		WStr_Name = m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name;
		
		cc = WStr_Name; 
		if (code == KOR_CODE) { JPHan_2_KRHan (&cc); }
		//***미리 약간 손을 써 두기로 한다

		if (code == JAP_CODE) { tmpCStr = U2J(cc.GetBuffer()); tmpWStr = J2U(tmpCStr); }
		else if (code == KOR_CODE) { tmpCStr = U2K(cc.GetBuffer()); tmpWStr = K2U(tmpCStr); }
		WStr_Name_chg = tmpWStr; free (tmpCStr); free (tmpWStr);
		//*문자열 변환으로 언어에 호환 가능한지 체크

		if (WStr_Name != WStr_Name_chg) {

			Change_FDName_Dialog dlg;

			if (cc == WStr_Name_chg) { goto FILE_LABEL_2; }
			//*이때는 다이얼로그 호출 없이 그대로 넘어가도 된다

			WStr_FullPath.Format (_T("%s : \n코드페이지에서 파일명을 인식할 수 없습니다.\n깨지는 문자열을 바꾸세요."), 
				WStr_Name);
			AfxMessageBox (WStr_FullPath);
			//*바뀐 문자열과 원래 문자열이 다르면 이름을 바꾸라고 메시지박스를 하나 띄운다

			dlg.Input_WStr = WStr_Name;
			dlg.Output_WStr = WStr_Name_chg;
			dlg.code = code;
			dlg.Meta_type = FILE_DB;
			dlg.ShouldChange = true;
			//*다이얼로그 호출 전 준비

FILE_LABEL:
			if (dlg.DoModal() == IDOK) {
				
				WStr_Name_chg = dlg.Output_WStr;
				
FILE_LABEL_2:
				cc = m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath;
				cc = cc.Right (cc.GetLength() - Root_Dir.GetLength());
				cc.Replace (WStr_Name, WStr_Name_chg);
				for (unsigned int j = 0;j < m_Text_Idx_In_Dir.File_Num;j++) {
					WStr_FullPath = m_Text_Idx_In_Dir.File_Idx_Arr[j].File_FullPath;
					WStr_FullPath = WStr_FullPath.Right (WStr_FullPath.GetLength() - Root_Dir.GetLength());
					if ((j != i) && (cc == WStr_FullPath)) {
						AfxMessageBox (_T("파일명이 중복됩니다.\n다시 입력하세요."));
						goto FILE_LABEL;
					}
				}
				//***현재 존재하는 파일명들과 중복 체크를 해야 한다, 이건 전체 경로 포함이라 더 어려운듯

				cc = m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath;
				cc = cc.Left (cc.GetLength() - (unsigned int)wcslen(m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name));
				cc += WStr_Name_chg;
				if (code == KOR_CODE) { JPHan_2_KRHan (&WStr_Name_chg); JPHan_2_KRHan (&cc); }
				FD_Rename (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, cc.GetBuffer());
				//***파일명 바꿔주기

				memset (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name, 0, MAX_PATH);
				memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_Name, WStr_Name_chg.GetBuffer(), 
					(sizeof(TCHAR) * WStr_Name_chg.GetLength()));
				memset (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, 0, MAX_PATH);
				if (i == 458) { 
					int s = 0; 
				}
				memcpy (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, cc.GetBuffer(), 
					(sizeof(TCHAR) * cc.GetLength()));
				//***버퍼 바꿔주기

				CString sfsf; sfsf.Format (_T("%d : %s"), i, WStr_Name_chg);
				m_G_ListCtrl_Files->SetItemText (i, 0, sfsf);
				//*파일 리스트에 등록 따로 해주기

				for (unsigned int j = 0;j < (unsigned int)m_G_ListCtrl_Text->GetItemCount();j++) {

					Get_Fidx_Tidx (j, &fidx, &tidx);

					if ((tidx != 0) && (m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].File_Str_Idx == i)) {

						List_str = m_G_ListCtrl_Text->GetItemText (j, 0);
						List_str.Replace (WStr_Name, WStr_Name_chg);
						if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].Is_Text_Dirty) {
							m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Text_Idx_Arr[tidx-1].Is_Text_Dirty = true;
							List_str.Insert(Prefix_Length, T_Head_Dirty);
						}
						m_G_ListCtrl_Text->SetItemText (j, 0, List_str);
						//*텍스트 바꿔주기, Dirty 상태 고려

						if (!m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty) {
							m_Text_Idx_In_Dir.File_Idx_Arr[fidx].Is_FileTxt_Dirty = true;
							List_str = m_G_ListCtrl_Text->GetItemText (Get_List_Index(fidx,0)-1, 0);
							List_str.Insert (0, T_Head_Dirty);
							m_G_ListCtrl_Text->SetItemText (Get_List_Index(fidx,0)-1, 0, List_str);
						}
						//*파일 헤더 바꿔주기, Dirty 상태 고려

						if (!m_Text_Idx_In_Dir.Is_Dirty) { m_Text_Idx_In_Dir.Is_Dirty = true; }
						//*전체 Dirty 설정

					}

				}
				//*파일 문자열 중 해당 파일을 인덱스로 갖고 있는 경우는 싸그리 바꾸기

			}

		}
		//*언어변환을 적용한 문자열과 원래 문자열 비교

	}
	//***이후 파일 체크(Filename_Str도 바꾸기)

	for (unsigned int i = 0;i < Dir_Num;i++) { free (Dir_List[i]); } free (Dir_List);
	//*디렉토리 리스트 버퍼 해제

	ChkDlg->SetDlgItemText(IDC_CHK_FD_TEXT, _T("Check File/Directory Finished."));
	ChkDlg->m_Check_FDName_Progress.SetPos (100);
	//*마지막 텍스트 및 프로그레스 바 세팅

}








void Load_Hanja()
{
	JapHanja = KorHanja = NULL;
	HanjaCnt = 0;
	//*미리 초기화하기

	HANDLE hHanja = CreateFileW (L"Hanja.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHanja == INVALID_HANDLE_VALUE) {
		MessageBoxW (NULL, L"Hanja.txt 파일이 없습니다.\n이 파일이 없으면 파일 이름을 수동으로 지정해야 합니다.", L"Alert", MB_OK);
		return;
	}

	unsigned int HanjaListSize = (GetFileSize(hHanja, NULL)/sizeof(wchar_t)) - 1;
	wchar_t *buff = (wchar_t*)malloc ((HanjaListSize+1) * sizeof(wchar_t));
	memset (buff, 0, ((HanjaListSize+1) * sizeof(wchar_t)));
	SetFilePointer (hHanja, sizeof(wchar_t), NULL, FILE_BEGIN);
	ReadFile (hHanja, buff, (HanjaListSize * sizeof(wchar_t)), NULL, NULL);
	CloseHandle (hHanja);
	//*파일 읽어들이기

	for (unsigned int i = 0;i < HanjaListSize - 3;i++) {
		if ((buff[i] == '<') && (buff[i+1] == '-') && (buff[i+2] == '>')) { HanjaCnt++; }
	}
	//*로드하기

	KorHanja = (wchar_t*)malloc (sizeof(wchar_t) * HanjaCnt);
	memset (KorHanja, 0, (sizeof(wchar_t) * HanjaCnt));
	JapHanja = (wchar_t*)malloc (sizeof(wchar_t) * HanjaCnt);
	memset (JapHanja, 0, (sizeof(wchar_t) * HanjaCnt));
	//*할당 및 초기화

	unsigned int buffpntidx = 0;
	for (unsigned int i = 0;i < HanjaCnt;i++) {
		while (!((buff[buffpntidx] == '<') && (buff[buffpntidx+1] == '-') && (buff[buffpntidx+2] == '>'))) { buffpntidx++; }
		JapHanja[i] = buff[buffpntidx - 2]; KorHanja[i] = buff[buffpntidx + 4];
		buffpntidx++;
	}
	//*버퍼에서 얻어와 뿌리기

	free (buff);
	//*버퍼 해제
}


void Release_Hanja ()
{
	if (JapHanja != NULL) { free (JapHanja); }
	if (KorHanja != NULL) { free (KorHanja); }
}


void JPHan_2_KRHan (CString *str)
{
	for (unsigned int j = 0;j < (unsigned int)str->GetLength();j++) {
		if (str->GetBuffer()[j] == 0x30FC){ str->GetBuffer()[j] = '-'; }
		else if (str->GetBuffer()[j] == 0x3005){ 
			if (j > 0) { str->GetBuffer()[j] = str->GetBuffer()[j-1]; } 
			else { str->GetBuffer()[j] = '-'; }
		}
		else {
			for (unsigned int k = 0;k < HanjaCnt;k++) {
				if (str->GetBuffer()[j] == JapHanja[k]){ str->GetBuffer()[j] = KorHanja[k]; break; }
			}
			//*매칭되는 한자를 찾아서 바꿔치기
		}
	}
}
//*바꿀 수 있는 한자는 모두 한국한자로 바꾸는 함수