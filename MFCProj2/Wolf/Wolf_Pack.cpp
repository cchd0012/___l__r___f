#include "stdafx.h"

#include "Wolf_Pack.h"


extern CString Wolf_File_List;


int Set_FileOrDirName_Data (unsigned char *FileDirnames_Raw, unsigned int *FileDirnames_Pnt_Addr,
	unsigned int Dir_Idx, FOD_IN_DIR *Dir_Str_MetaCnt_Arr, unsigned int Dir_Idx_Size,
	FILE_METADATA *Metadata_Collect, unsigned int code, wchar_t ***Dir_File_Wstr_Pnt);
//*순수 [파일명+체크섬] 데이터 세팅 함수


int Set_Big_FileOrDirName_Data (unsigned char *FileDirnames_Raw, unsigned int *FileDirnames_Pnt_Addr,
	unsigned int Dir_Idx, FOD_IN_DIR *Dir_Str_MetaCnt_Arr, unsigned int Dir_Idx_Size,
	FILE_BIG_METADATA *Metadata_Collect, unsigned int code, wchar_t ***Dir_File_Wstr_Pnt);
//*순수 [파일명+체크섬] 데이터 세팅 함수(64bit wolf 파일 전용)





void creating (Pack_Dialog* PakDlg, unsigned int type, unsigned int code)	
{
	char *List_Buff, *List_Buff_Pnt;			//리스트 내용을 저장할 버퍼 및 포인터
	unsigned int List_Size;						//리스트의 크기

	WOLF_HEADER Dxa_Head;						//wolf 파일의 헤더

	unsigned char *Files_Buff;					//파일들을 한데 모아놓은 버퍼
	unsigned char *Each_File_Buff;				//각 파일의 버퍼
	unsigned int FilesBuff_Size;				//파일 버퍼의 크기
	unsigned int Each_File_Size;				//각 파일의 크기
	unsigned int Offset_Acumm;					//오프셋 누적치를 계산해 반영하기 위한 변수
	
	unsigned char *FileDirnames_Raw;			//[파일이름+체크섬] 배열을 곧바로 저장 불가하므로 파일이름 정보로 변환한다.
	unsigned int FileDirnames_Raw_Size;			//순수 [파일이름+체크섬]의 크기

	FILE_METADATA *Metadata_Collect;			//파일의 메타데이터를 저장할 곳
	unsigned int Metadata_Collect_Size;			//파일이름 토큰들을 저장할 때 그 갯수

	DIR_IDX *Dir_Idx_Collect;					//디렉토리의 정보를 저장할 곳
	unsigned int Dir_Idx_Collect_Size;			//디렉토리 정보를 저장할 때 그 갯수

	FOD_IN_DIR *Dir_Str_MetaCnt_Arr;			//디렉토리가 소유한 메타데이터 수 및 디렉토리 이름 포인터를 저장하는 배열
	unsigned int Dir_Idx_Tmp;					//임시용으로 디렉토리 인덱스를 지정할 변수
	unsigned int FoD_Idx_Tmp;					//임시용으로 디렉토리가 소유한 [파일/디렉토리] 인덱스를 지정할 변수

	unsigned char *Total_Meta_Buff;				//위 3개를 한데 모아 저장할 곳
	unsigned int Total_Meta_Size;				//위 3개를 총합한 크기

	char Parent_Dir [MAX_PATH];					//부모 디렉토리의 경로를 저장할 변수
	char *Each_Str;								//각 파일/디렉토리 문자열 포인터
	unsigned int FileOrDir_Length;				//파일/디렉토리의 이름 길이
	unsigned int Each_Str_Len;					//각 파일/디렉토리 문자열의 길이

	HANDLE hEachFile;							//각 파일을 기록할 때 쓰이는 핸들
	HANDLE hDxa;								//작업을 마친 후 새 wolf 파일을 기록할 때 쓰이는 핸들


	CString tmpCstr;							//있으면 유용하게 써먹을 수 있다
	wchar_t *List_Buff_Backup;					//*경로를 여는 데 쓰일 백업버퍼
	unsigned int Uni_List_Size;					//*위 백업버퍼의 크기
	wchar_t ***Dir_File_Wstr_Pnt;				//*경로를 지정하는 포인터
		

	DWORD lp_read, lp_write;


	TCHAR dxaname[MAX_PATH];
	memset (dxaname, 0, sizeof(TCHAR) * MAX_PATH);
	PakDlg->GetDlgItemText (IDC_P_EDIT1, dxaname, MAX_PATH);
	//*파일 이름 얻어오기

	if (code == JAP_CODE) { List_Buff = U2J (Wolf_File_List.GetBuffer()); }
	else if (code == KOR_CODE) { List_Buff = U2K (Wolf_File_List.GetBuffer()); }
	List_Size = (unsigned int)strlen(List_Buff);

	List_Buff_Backup = Wolf_File_List.GetBuffer();
	Uni_List_Size = (unsigned int)wcslen(List_Buff_Backup);
	//*List_Buff 변수는 문자열을 긁어와 계산할 때 쓰일 변수고,
	//*List_Buff_Backup은 디렉토리/파일을 열 때 쓰일 변수다
	//*그러므로 List_Buff는 지정된 언어코드에 따라 바꾼다.
	//***0. 리스트 읽어오기


	
	Dir_Idx_Collect_Size = 0;
	for (unsigned int i = 0;i < List_Size-0x3;i++) {
		if (*(unsigned int*)(&List_Buff[i]) == 0x0A0D0A0D) { Dir_Idx_Collect_Size++; }
	}
	//*루트 디렉토리를 포함한 총 디렉토리 수 추산. 개행이 2번 연달아 있는 횟수를 추산하면 된다

	Dir_Str_MetaCnt_Arr = (FOD_IN_DIR*)malloc(sizeof(FOD_IN_DIR)*Dir_Idx_Collect_Size);
	memset (Dir_Str_MetaCnt_Arr, 0, sizeof(FOD_IN_DIR)*Dir_Idx_Collect_Size);
	//*디렉토리가 가진 메타데이터 수를 추산해 저장하는 배열 생성 및 초기화

	Dir_Str_MetaCnt_Arr[0].Dir_Str = List_Buff;
	//*마침 제일 처음에 있는 게 루트 디렉토리므로 이렇게 하면 된다

	Metadata_Collect_Size = 1;
	Dir_Idx_Tmp = 0;
	for (unsigned int i = 0;i < List_Size-4;i++) {
		if (List_Buff[i] == 0x9) { Metadata_Collect_Size++; Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt++; }
		//*탭을 만났을 때. 메타데이터 배열수 뿐만 아니라 각 디렉토리 소유 메타데이터 수도 늘려줌

		else if ((List_Buff[i] == 0xD) && (List_Buff[i+1] == 0xA)) { 
			if ((List_Buff[i+2] == 0xD) && (List_Buff[i+3] == 0xA)) { 
				Dir_Idx_Tmp++; 
				Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Str = &List_Buff[i+4];
			}
			//*개행이 연속 2번일 때 -> 다음 디렉토리로 넘어감을 뜻함. 그러므로 그 시작지점을 문자열로 삼기로 한다
		}
		//*개행을 만났을 때 그 부분을 아예 0으로 만들어 버려야 하지만, 밑의 과정 때문에 바로 그러지는 않는다
	}
	//*루트 디렉토리를 포함한 총 메타데이터 수 추산. 탭의 수를 기준으로 추산하면 되지만 루트 디렉토리는 포함 안되므로 1을 더 더한다
	//*추가로 개행 부분을 0으로 만들어버려서 간단하게 파일/디렉토리에 접근 가능한 문자열을 만들고, 해당 디렉토리가 가진 메타데이터를 늘려야 한다

	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {
		Dir_Str_MetaCnt_Arr[i].FileOrDir_In_Dir_Arr = 
			(char**)malloc(sizeof(char*) * Dir_Str_MetaCnt_Arr[i].Dir_Meta_Cnt);
		memset (Dir_Str_MetaCnt_Arr[i].FileOrDir_In_Dir_Arr, 0, sizeof(char*) * Dir_Str_MetaCnt_Arr[i].Dir_Meta_Cnt);
	}
	//*문자열 포인터 배열 할당 및 초기화
	


	Dir_File_Wstr_Pnt = (wchar_t***)malloc(sizeof(wchar_t**) * Dir_Idx_Collect_Size);
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {
		Dir_File_Wstr_Pnt[i] = (wchar_t**)malloc (sizeof(wchar_t*) * (Dir_Str_MetaCnt_Arr[i].Dir_Meta_Cnt + 1));
	}
	Dir_File_Wstr_Pnt[0][0] = List_Buff_Backup;
	Dir_Idx_Tmp = 0;
	for (unsigned int i = 0;i < Uni_List_Size-4;i++) { 
		if ((List_Buff_Backup[i] == 0xD) && (List_Buff_Backup[i+1] == 0xA) 
			&& (List_Buff_Backup[i+2] == 0xD) && (List_Buff_Backup[i+3] == 0xA)) { 
				Dir_Idx_Tmp++;
				Dir_File_Wstr_Pnt[Dir_Idx_Tmp][0] = &List_Buff_Backup[i+4];
		} 
	}
	//***경로에 접근하기 위한 포인터도 추가 할당, +1은 해당 덩어리에 대응하는 디렉토리를 표시해야 하기 때문



	Dir_Idx_Tmp = FoD_Idx_Tmp = 0;
	for (unsigned int i = 0;i < List_Size-1;i++) {
		if (List_Buff[i] == 0x9) { 
			Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp] = &List_Buff[i+1];
			FoD_Idx_Tmp++;
		}
		//*탭을 만났을 때. 이번에는 각각의 문자열들을 제대로 매칭시켜준다(물론 탭은 넘기고)

		else if ((List_Buff[i] == 0xD) && (List_Buff[i+1] == 0xA)) { 
			List_Buff[i] = List_Buff[i+1] = 0; 
			if ((List_Buff[i+2] == 0xD) && (List_Buff[i+3] == 0xA)) { 
				FoD_Idx_Tmp = 0;Dir_Idx_Tmp++; 
				if ((List_Buff[i+4] == 0xD) && (List_Buff[i+5] == 0xA)) { 
					i--; while ((List_Buff[i] != 0) && (i >= 0)) { i--; }
					if (code == JAP_CODE) { tmpCstr = J2U (&List_Buff[i+1]); }
					else if (code == KOR_CODE) { tmpCstr = K2U (&List_Buff[i+1]); }
					AfxMessageBox (tmpCstr + _T(" : 디렉토리가 비었습니다.\n리스트에서 삭제해주세요."));
					for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
						free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
						free (Dir_File_Wstr_Pnt[jj]);
					}
					free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt); tmpCstr.Empty();
					return;
				}
			}
			//*개행이 연속 2번일 때 -> 다음 디렉토리로 넘어감을 뜻함.
			//*그러므로 [파일/디렉토리] 인덱스를 초기화하고 디렉토리 인덱스를 더한다
		}
		//*이번에야말로 개행 부분을 0으로 만든다
	}
	//*각 디렉토리 소유 [파일/디렉토리] 배열 세팅



	Dir_Idx_Tmp = FoD_Idx_Tmp = 0;
	for (unsigned int i = 0;i < Uni_List_Size-1;i++) {
		if (List_Buff_Backup[i] == 0x9) { 
			Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1] = &List_Buff_Backup[i+1];
			FoD_Idx_Tmp++;
		}
		//*탭을 만났을 때. 이번에는 각각의 문자열들을 제대로 매칭시켜준다(물론 탭은 넘기고)

		else if ((List_Buff_Backup[i] == 0xD) && (List_Buff_Backup[i+1] == 0xA)) { 
			List_Buff_Backup[i] = List_Buff_Backup[i+1] = 0; 
			if ((List_Buff_Backup[i+2] == 0xD) && (List_Buff_Backup[i+3] == 0xA)) 
			{ FoD_Idx_Tmp = 0;Dir_Idx_Tmp++; }
			//*개행이 연속 2번일 때 -> 다음 디렉토리로 넘어감을 뜻함.
			//*그러므로 [파일/디렉토리] 인덱스를 초기화하고 디렉토리 인덱스를 더한다
		}
		//*이번에야말로 개행 부분을 0으로 만든다
	}
	//***접근을 위한 경로 포인터에도 추가로 세팅



	FileDirnames_Raw_Size = 0;
	FilesBuff_Size = 0;
	//*[파일이름+체크섬] 원데이터 크기 및 파일 버퍼 크기 초기화


	List_Buff_Pnt = List_Buff;
	Dir_Idx_Tmp = FoD_Idx_Tmp = 0;
	for (unsigned int i = 0;i < Metadata_Collect_Size;i++) {

		if (i == 0) { FileDirnames_Raw_Size += (sizeof(short) * 2); }
		//*루트 디렉토리를 상징하는데, 길이가 아예 없으므로 4만 더한다

		else {

			if (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt == 0) { 
				FoD_Idx_Tmp = 0; Dir_Idx_Tmp++; continue; 
			}

			while (*List_Buff_Pnt != 0x9) { List_Buff_Pnt++; } List_Buff_Pnt++;
			//*탭 바로 다음곳을 찾기(경로의 시작)

			FileOrDir_Length = (unsigned int)strlen(List_Buff_Pnt) - (Get_Last_SlashPos(List_Buff_Pnt, code)+1);
			FileOrDir_Length += (4-(FileOrDir_Length%4));
			//*마지막 슬래시 너머의 문자열 길이만 얻어내서 4로 정렬 후 더해야 한다

			FileDirnames_Raw_Size += ((sizeof(short) + FileOrDir_Length) * 2);
			//*(길이*2) + 4만큼 더한다

			if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == 1) { 
				hEachFile = CreateFileW (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				Each_File_Size = GetFileSize (hEachFile, NULL);
				FilesBuff_Size += Each_File_Size + (4-(Each_File_Size%4))%4;
				CloseHandle (hEachFile);
				//*파일일 때 파일 크기 더하기
				//*이건 압축하지 않았을 때의 크기이고, 어차피 압축하게 된다 한들 크기면에서 벗어나지 않기에
				//*이러는 게 제일 낫다. 기록할 때는 FilesBuff_Size 만큼만 하면 된다
			}
			else if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == -1) {
				tmpCstr = Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]; 
				AfxMessageBox (tmpCstr + _T(" : 해당 파일/디렉토리가 없습니다."));
				for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
					free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
					free (Dir_File_Wstr_Pnt[jj]);
				} 
				free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
				return;
			}
			else if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == 0) 
			{;}
			//*덤으로 파일 버퍼의 크기도 확정짓는다.(신버전에서는 파일 버퍼 오프셋도 4로 나눠떨어져야 한다)

			FoD_Idx_Tmp++;
			if (FoD_Idx_Tmp >= Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt) 
			{ FoD_Idx_Tmp = 0; Dir_Idx_Tmp++; }
			//*갱신 방향을 잘못 잡을 뻔했음

		}
		//*다른 파일 or 디렉토리

	}
	//*[파일명+체크섬] 길이 계산 시, 파일 이름 길이는 4로 올려 정렬해 계산한다
	//*여기서 덤으로 파일 버퍼에 필요한 길이도 계산한다

	Files_Buff = (unsigned char*)malloc(sizeof(char)*FilesBuff_Size);
	if (Files_Buff == NULL) {
#ifdef _WIN64
		AfxMessageBox (_T(" : 용량이 지나치게 커서 합성할 수 없습니다."));
#else
		AfxMessageBox (_T(" : 총합 용량이 너무 큽니다. 64비트로 돌리십시오."));
#endif
		return;
	}
	memset (Files_Buff, 0, FilesBuff_Size);
	//*파일 버퍼 활성화 및 초기화

	Total_Meta_Size = FileDirnames_Raw_Size + 
		sizeof(FILE_METADATA)*Metadata_Collect_Size + 
		sizeof(DIR_IDX)*Dir_Idx_Collect_Size;
	Total_Meta_Buff = (unsigned char*)malloc(sizeof(char)*Total_Meta_Size);
	memset (Total_Meta_Buff, 0, Total_Meta_Size);
	//*총합 메타데이터 공간 계산 후 할당 및 초기화

	FileDirnames_Raw = Total_Meta_Buff;
	Metadata_Collect = (FILE_METADATA*)(Total_Meta_Buff + FileDirnames_Raw_Size);
	Dir_Idx_Collect = (DIR_IDX*)(Total_Meta_Buff + FileDirnames_Raw_Size + sizeof(FILE_METADATA)*Metadata_Collect_Size);
	//*각 위치에 포인터 세팅

	//***1. 루트 디렉토리를 포함해 총 [파일이름+체크섬] 배열 원소 수, 파일 버퍼 크기, 디렉토리 인덱스 수, 총 메타데이터 수 추산





	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {

		Dir_Idx_Collect[i].mt_number = (Dir_Str_MetaCnt_Arr[i].Dir_Meta_Cnt);
		//*각 디렉토리가 소유하는 메타데이터 수를 미리 세놓았으므로 대입해 준다

		if (i == 0) {
			Dir_Idx_Collect[i].dirmt_ofs = 0x0;
			//*루트 디렉토리 메타데이터 맨 처음에 있으므로 당연히 0을 넣어야 한다
			Dir_Idx_Collect[i].parent_dir = -1;
			//*루트 디렉토리 부모 디렉토리는 없으므로 -1
			Dir_Idx_Collect[i].mt_offset = (sizeof(FILE_METADATA));
			//*루트 디렉토리가 소유한 메타데이터는 루트 디렉토리 메타데이터 다음에 있으므로 그 크기만큼만 더해주면 된다
		}
		//*루트 디렉토리 세팅

		else {
			memset (Parent_Dir, 0, MAX_PATH);
			memcpy (Parent_Dir, 
				Dir_Str_MetaCnt_Arr[i].Dir_Str, 
				Get_Last_SlashPos (Dir_Str_MetaCnt_Arr[i].Dir_Str, code));
			//*마지막 슬래시 이전까지만 문자열을 복사하면 그게 부모 디렉토리가 된다

			Dir_Idx_Collect[i].parent_dir = -1;
			for (Dir_Idx_Tmp = 0 ; Dir_Idx_Tmp < i ; Dir_Idx_Tmp++) {
				if (strcmp (Parent_Dir, Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Str) == 0) {
					Dir_Idx_Collect[i].parent_dir = (sizeof(DIR_IDX) * Dir_Idx_Tmp);
					break;
				}
			}
			if (Dir_Idx_Tmp == i) {
				tmpCstr = Dir_File_Wstr_Pnt[i][0];
				AfxMessageBox (tmpCstr + _T(" : 부모 디렉토리가 없습니다.\n리스트가 깨진 것 같습니다."));
				for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
					free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
					free (Dir_File_Wstr_Pnt[jj]);
				}
				free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
				free (Files_Buff); free (Total_Meta_Buff);
				return;
				//*만일 여기에 이르러서까지 부모 디렉토리가 없다고 나오면 분명 잘못된 것이다
			}
			//*따라서 그것을 앞서 선언한 Dir_Str_MetaCnt_Arr로부터 문자열 포인터를 얻어 비교하면 된다(부모 디렉토리 세팅)
			//*Dir_Idx_Tmp는 현재 세팅 중인 디렉토리의 부모 디렉토리 인덱스를 가지게 된다

			Dir_Idx_Collect[i].mt_offset = (sizeof(FILE_METADATA));
			//*루트 디렉토리 메타데이터 오차 보정
			for (unsigned int j = 0;j < i;j++) {
				Dir_Idx_Collect[i].mt_offset += (sizeof(FILE_METADATA) * Dir_Str_MetaCnt_Arr[j].Dir_Meta_Cnt);
			}
			//*그냥 [이전 디렉토리까지의 총 메타데이터 수] + [루트 디렉토리의 메타데이터] 잘 더하면 될 것이다
			//*디렉토리 소유 메타데이터 오프셋 세팅

			for (FoD_Idx_Tmp = 0 ; FoD_Idx_Tmp < Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt ; FoD_Idx_Tmp++) {
				if (strcmp (Dir_Str_MetaCnt_Arr[i].Dir_Str, 
					Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp]) == 0) {
					break;
				}
				//*현재 디렉토리 문자열과 부모 디렉토리 내 [파일/디렉토리] 문자열들과 비교
			}
			if (FoD_Idx_Tmp == Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt) {
				tmpCstr = Dir_File_Wstr_Pnt[Dir_Idx_Tmp][0];
				tmpCstr += _T("\n");
				tmpCstr += Dir_File_Wstr_Pnt[i][0];
				tmpCstr += _T(" 디렉토리 내에 해당 디렉토리가 없습니다.\n리스트가 깨진 것 같습니다.");
				AfxMessageBox (tmpCstr);
				for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
					free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr);
					free (Dir_File_Wstr_Pnt[jj]);
				}
				free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
				free (Files_Buff); free (Total_Meta_Buff);
				return;
				//*만일 얻어내지 못했다면 에러처리
			}
			//*부모 디렉토리의 메타데이터 오프셋은 알고 있으므로 (루트 디렉토리는 0x0, 이하 디렉토리들은 위에서 미리 구한다)
			//*부모 디렉토리로부터 순차적으로 내려가서 어디서 문자열이 '같은지' 파악하면 된다
			Dir_Idx_Collect[i].dirmt_ofs = (Dir_Idx_Collect[Dir_Idx_Tmp].mt_offset
				 + ((sizeof(FILE_METADATA) * FoD_Idx_Tmp)));
			//*그러면 (부모 디렉토리의 오프셋) + (메타데이터 원소크기*내려간 수)가 될 것이다
			//*디렉토리 오프셋(in 메타데이터) 세팅
			//*FoD_Idx_Tmp는 부모 디렉토리 내에 해당 디렉토리가 있는 인덱스를 가리킨다
		}
		//*다른 디렉토리 세팅
	}
	//***2. [디렉토리 인덱스] 배열 세팅. 어차피 메타데이터가 순차적으로 세팅되어 있으므로 순서대로만 잘 하면 된다





	Dir_Idx_Tmp = (sizeof(short)*2);
	if (Set_FileOrDirName_Data (FileDirnames_Raw, &Dir_Idx_Tmp, 0, Dir_Str_MetaCnt_Arr, 
		Dir_Idx_Collect_Size, Metadata_Collect, code, Dir_File_Wstr_Pnt) == -1)
	{
		tmpCstr = dxaname; AfxMessageBox (tmpCstr + _T(" : 메타데이터 구성을 실패했습니다.\n"));
		for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
			free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
			free (Dir_File_Wstr_Pnt[jj]);
		}
		free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt); 
		free (Files_Buff); free (Total_Meta_Buff);
		return;
		//*-1이 반환됐다면 에러처리
	}
	//***3. [파일명+체크섬] 배열 세팅. 이 때 Dir_Str_MetaCnt_Arr를 참조한다
	//***(인자 : FileDirnames_Raw, FileDirnames_Collect 인덱스(포인터), Dir_Str_MetaCnt_Arr, 
	//***디렉토리 인덱스의 인덱스(Dir_Idx_Tmp역할), Metadata_Collect), 코드값 -> int값 반환(-1이 에러)
	//***어차피 루프 디렉토리는 그냥 0이니까 미리 4바이트를 넘기고 시작한다






	Dir_Idx_Tmp = FoD_Idx_Tmp = 0;
	Offset_Acumm = 0;
	for (unsigned int i = 0;i < Metadata_Collect_Size;i++) {

		PakDlg->m_Pack_Progress.SetPos (PROGRESSVALUE(i, Metadata_Collect_Size));
		//*프로그레스 바 세팅

		Metadata_Collect[i].CompSize = -1;
		//*패딩으로 처리되서 어쩔 수가 없는듯

		if (i == 0) {
			Metadata_Collect[i].fltok_start = 0x0;
			Metadata_Collect[i].attr = (DIR);
			Metadata_Collect[i].generated = Metadata_Collect[i].last_modified = 
				Metadata_Collect[i].last_modified = 0x0;
			Metadata_Collect[i].offset = Metadata_Collect[i].UncompSize = 0x0;
		}
		//*루트 디렉토리일 때. 디렉토리고, 생성, 마지막 수정, 마지막 접속 같은 건 의미가 없으므로 0 처리한다
		//*물론 오프셋, 비압축사이즈, 압축사이즈도 의미가 없다

		else {

			if (Is_File_Or_Dir(Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == 0) {
				Metadata_Collect[i].attr = DIR;		//*속성 : 디렉토리

				hEachFile = CreateFileW (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				GetFileTime(hEachFile, 
					(LPFILETIME)&(Metadata_Collect[i].generated), 
					(LPFILETIME)&(Metadata_Collect[i].last_accessed),
					(LPFILETIME)&(Metadata_Collect[i].last_modified));
				CloseHandle (hEachFile);
				//*생성, 접속 시간, 수정 시간 반영

				Metadata_Collect[i].UncompSize = 0x0;
				//*비압축크기 0, 압축크기 -1로 처리

				Metadata_Collect[i].offset = 0x0;
				for (unsigned int j = 0;j < Dir_Idx_Collect_Size;j++) {
					if (strcmp(Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp], 
						Dir_Str_MetaCnt_Arr[j].Dir_Str) == 0) {
							Metadata_Collect[i].offset = (sizeof(DIR_IDX) * j); break;
					}
				}
				//*오프셋의 값으로 디렉토리 인덱스 내의 오프셋을 반영해야 한다
			}
			//*디렉토리일 때. 생성, 수정, 접속시간, 비압축사이즈, 압축사이즈의 의미가 없다

			else if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == 1) {


				if (!CanBeFile (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp], code)) {
					int ll = Get_Last_SlashPos (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp], code);
					if (code == JAP_CODE) { tmpCstr = J2U (&Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp][ll + 1]); }
					else if (code == KOR_CODE) { tmpCstr = K2U (&Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp][ll + 1]); }
					AfxMessageBox (tmpCstr + _T(" : 언어코드가 달라 파일을 합성할 수 없습니다.\n언어코드를 바꿔보십시오.")); tmpCstr.Empty();
					for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
						free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
						free (Dir_File_Wstr_Pnt[jj]);
					}
					free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
					free (Files_Buff); free (Total_Meta_Buff);
					return;
				}
				//***파일 이름이 될 문자열 체크 : 물음표가 끼어 있으면 나중에 출력을 할 수 없으므로 에러처리한다.
				//***체크 조건 : 마지막 슬래시까지 찾아가다가 찾기 전에 물음표 나오면 불가능


				tmpCstr = _T("Packing : ");
				PakDlg->SetDlgItemText (IDC_P_TEXT, tmpCstr + Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]);
				//*출력 문자열 갱신

				Metadata_Collect[i].attr = FILE_DB;	//*속성 : 파일

				hEachFile = CreateFileW (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				Each_File_Size = GetFileSize (hEachFile, NULL);

				if (Each_File_Size != 0) { 
					Each_File_Buff = (unsigned char*)malloc(sizeof(char) * Each_File_Size * 2); 
					memset (Each_File_Buff, 0, Each_File_Size * 2);
					ReadFile (hEachFile, Each_File_Buff, Each_File_Size, &lp_read, NULL);
				}
				else { Each_File_Buff = NULL; }

				Metadata_Collect[i].UncompSize = Each_File_Size;
				//*파일 핸들 열고 읽어오기, 비압축 크기 반영, 보정치 확정

				GetFileTime(hEachFile, 
					(LPFILETIME)&(Metadata_Collect[i].generated), 
					(LPFILETIME)&(Metadata_Collect[i].last_accessed),
					(LPFILETIME)&(Metadata_Collect[i].last_modified));
				CloseHandle (hEachFile);
				//*생성, 접속 시간, 수정 시간 반영

				Each_Str = Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp];
				Each_Str_Len = (unsigned int)strlen(Each_Str);

				//***파일 크기가 0일 때도 필터를 걸어서 처리해 준다

				if ((strcmp(Each_Str + Each_Str_Len - strlen(".common"), ".common") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - strlen(".dat"), ".dat") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - strlen(".project"), ".project") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - strlen(".xxxxx"), ".xxxxx") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - strlen(".mps"), ".mps") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - strlen(".mid"), ".mid") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - strlen(".tile"), ".tile") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - strlen(".txt"), ".txt") == 0)) {

					if (strcmp(Each_Str + Each_Str_Len - strlen("Game.dat"), "Game.dat") == 0) {
						Metadata_Collect[i].CompSize = -1;
						//*예외로 Game.dat만은 압축저장하지 않는다
					}
					else {
						if (Each_File_Size != 0) {
							Encode (Each_File_Buff, Each_File_Size);
							Metadata_Collect[i].CompSize = (*(unsigned int*)(&Each_File_Buff[4]));
							Each_File_Size = (unsigned int)Metadata_Collect[i].CompSize;
							//*압축 완료된 버퍼 내에서 압축된 크기 확인
						}
						else { Metadata_Collect[i].CompSize = -1; }
					}
				}
				else {
					Metadata_Collect[i].CompSize = -1;
					//*압축할 파일이 아니라면 압축크기를 -1로 잡아버린다
				}
				//*파일 종류에 따라 압축, 비압축을 설정한다
				//*만일 확장자가 .dat, .project, .xxxxx, .mps, .mid가 들어가있으면 이것들을 압축해 저장해야 한다

				Metadata_Collect[i].offset = Offset_Acumm;
				if (Each_File_Size != 0) { memcpy (Files_Buff + Offset_Acumm, Each_File_Buff, Each_File_Size); }
				Offset_Acumm += (Each_File_Size + (4 - (Each_File_Size%4))%4);
				//*오프셋 저장, 파일 버퍼에 복사, 오프셋 누적시키기

				if (Each_File_Size != 0) { free (Each_File_Buff); }
				//*파일 버퍼 해제
			}
			//*파일일 때. 어떤 파일들은 압축할 필요가 있다.

			else {
				tmpCstr = Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1];
				if (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp] == NULL) {
					tmpCstr += _T(" : 해당 디렉토리는 다른 파일 혹은 디렉토리를 소유하지 않습니다.\n이 디렉토리를 리스트에서 제거한 후 다시 시도하십시오.");
				}
				else {
					tmpCstr += _T(" : 해당 문자열은 [파일/디렉토리]로서 존재하지 않습니다.");
				}
				AfxMessageBox (tmpCstr);

				for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
					free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
					free (Dir_File_Wstr_Pnt[jj]);
				}
				free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
				free (Files_Buff); free (Total_Meta_Buff);
				return;
			}
			//*해당 문자열이 디렉토리나 파일의 경로가 아닐 때 -> 에러

			FoD_Idx_Tmp++;
			if (FoD_Idx_Tmp == (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt)) {
				FoD_Idx_Tmp = 0; Dir_Idx_Tmp++;
			}
			//*만일 한 디렉토리 내의 메타데이터 및 파일을 전부 반영했다면,
			//*다음 디렉토리로 넘어가기 위해 적절하게 초기화한다

			//*[디렉토리 인덱스] 순으로 맞추어 정리한다
			//*메타데이터는 순차적으로 맞추어 정리해 나가기에 위와 같이 단순하게 해도 문제없다
		}
		//*이외의 다른 [디렉토리/파일]일 때. 각 디렉토리의 메타데이터 소유 수를 참조하여 촤라락 정리한다
	}
	//*디렉토리(0x10) 아니면 파일(0x20)으로 일괄 정리하도록 한다. 순서대로 적용 가능하니 편할듯.
	//*fltok_start는 위 과정에서 세팅되므로 굳이 건들지 않아도 된다
	//*위 작업을 다 마치면 Offset_Acumm이 최종 파일 버퍼 크기가 된다

	//***4. [메타데이터] 배열 및 파일 버퍼 세팅, 이때 파일버퍼도 같이 세팅한다




	memset (&Dxa_Head, 0, sizeof(WOLF_HEADER));
	if (type == DXA_220) { Dxa_Head.magic = WOLF_MAGIC; }
	else if (type == DXA_THMK) { Dxa_Head.magic = DXA_MAGIC; }
	else { Dxa_Head.magic = WOLF_MAGIC; }
	Dxa_Head.total_toklen = Total_Meta_Size;
	Dxa_Head.header_size = (sizeof(WOLF_HEADER));
	Dxa_Head.totalmt_ofs = (Offset_Acumm + sizeof(WOLF_HEADER));
	Dxa_Head.filemt_ofs = (FileDirnames_Raw_Size);
	Dxa_Head.dir_hd_ofs = (Total_Meta_Size - (sizeof(DIR_IDX)*Dir_Idx_Collect_Size));

	unsigned char *pt;

	if (type == DXA_220) {
		pt = (unsigned char*)(&Dxa_Head);
		for (unsigned int i = 0;i < sizeof(WOLF_HEADER);i++) { pt[i] ^= WOLF_KEY2[(i)%KEY_SIZE]; }
		//*헤더 xor
		pt = Files_Buff;
		for (unsigned int i = 0;i < Offset_Acumm;i++) { pt[i] ^= WOLF_KEY2[(i + sizeof(WOLF_HEADER))%KEY_SIZE]; }
		//*파일버퍼 xor
		pt = Total_Meta_Buff;
		for (unsigned int i = 0;i < Total_Meta_Size;i++) { pt[i] ^= WOLF_KEY2[(i + sizeof(WOLF_HEADER) + Offset_Acumm)%KEY_SIZE]; }
		//*메타데이터 xor
	}
	if (type == DXA_THMK) {
		pt = (unsigned char*)(&Dxa_Head);
		for (unsigned int i = 0;i < sizeof(WOLF_HEADER);i++) { pt[i] ^= THMK2_KEY[(i)%KEY_SIZE]; }
		//*헤더 xor
		pt = Files_Buff;
		for (unsigned int i = 0;i < Offset_Acumm;i++) { pt[i] ^= THMK2_KEY[(i + sizeof(WOLF_HEADER))%KEY_SIZE]; }
		//*파일버퍼 xor
		pt = Total_Meta_Buff;
		for (unsigned int i = 0;i < Total_Meta_Size;i++) { pt[i] ^= THMK2_KEY[(i + sizeof(WOLF_HEADER) + Offset_Acumm)%KEY_SIZE]; }
		//*메타데이터 xor
	}
	else {
		pt = (unsigned char*)(&Dxa_Head);
		for (unsigned int i = 0;i < sizeof(WOLF_HEADER);i++) { pt[i] ^= WOLF_KEY[(i)%KEY_SIZE]; }
		//*헤더 xor
		pt = Files_Buff;
		for (unsigned int i = 0;i < Offset_Acumm;i++) { pt[i] ^= WOLF_KEY[(i + sizeof(WOLF_HEADER))%KEY_SIZE]; }
		//*파일버퍼 xor
		pt = Total_Meta_Buff;
		for (unsigned int i = 0;i < Total_Meta_Size;i++) { pt[i] ^= WOLF_KEY[(i + sizeof(WOLF_HEADER) + Offset_Acumm)%KEY_SIZE]; }
		//*메타데이터 xor
	}
	//***5. 헤더를 하나 잡고 올바른 값을 기술하고 한번에 모조리 xor 한다




	hDxa = CreateFileW (dxaname, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile (hDxa, &Dxa_Head, sizeof(WOLF_HEADER), &lp_write, NULL);
	WriteFile (hDxa, Files_Buff, Offset_Acumm, &lp_write, NULL);
	WriteFile (hDxa, Total_Meta_Buff, Total_Meta_Size, &lp_write, NULL);
	
	PakDlg->m_Pack_Progress.SetPos (100);
	PakDlg->SetDlgItemText (IDC_P_TEXT, _T("Packing Finished"));
	//***6. 헤더, 파일공간, 총합 메타데이터 순으로 파일을 기록한다




	CloseHandle (hDxa);
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) { 
		free (Dir_Str_MetaCnt_Arr[i].FileOrDir_In_Dir_Arr); 
		free (Dir_File_Wstr_Pnt[i]);
	}
	free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
	free (Files_Buff); free (Total_Meta_Buff);
	//***7. 전부 해제
}



void creating_64bit (Pack_Dialog* PakDlg, unsigned int type, unsigned int code)
{
	char *List_Buff, *List_Buff_Pnt;			//리스트 내용을 저장할 버퍼 및 포인터
	unsigned int List_Size;						//리스트의 크기

	WOLF_BIG_HEADER Dxa_Head;					//wolf 파일의 헤더

	unsigned int Xor_Ofs;						//메타데이터를 정상화시키기 위해 xor한 횟수를 저장한다

	unsigned char *Files_Buff;					//파일들을 한데 모아놓은 버퍼
	unsigned char *Each_File_Buff;				//각 파일의 버퍼
	unsigned int FilesBuff_Size;				//파일 버퍼의 크기
	unsigned int Each_File_Size;				//각 파일의 크기
	__int64 Offset_Acumm;						//오프셋 누적치를 계산해 반영하기 위한 변수
	
	unsigned char *FileDirnames_Raw;			//[파일이름+체크섬] 배열을 곧바로 저장 불가하므로 파일이름 정보로 변환한다.
	unsigned int FileDirnames_Raw_Size;			//순수 [파일이름+체크섬]의 크기

	FILE_BIG_METADATA *Metadata_Collect;		//파일의 메타데이터를 저장할 곳
	unsigned int Metadata_Collect_Size;			//파일이름 토큰들을 저장할 때 그 갯수

	DIR_BIG_IDX *Dir_Idx_Collect;				//디렉토리의 정보를 저장할 곳
	unsigned int Dir_Idx_Collect_Size;			//디렉토리 정보를 저장할 때 그 갯수

	FOD_IN_DIR *Dir_Str_MetaCnt_Arr;			//디렉토리가 소유한 메타데이터 수 및 디렉토리 이름 포인터를 저장하는 배열
	unsigned int Dir_Idx_Tmp;					//임시용으로 디렉토리 인덱스를 지정할 변수
	unsigned int FoD_Idx_Tmp;					//임시용으로 디렉토리가 소유한 [파일/디렉토리] 인덱스를 지정할 변수

	unsigned char *Total_Meta_Buff;				//위 3개를 한데 모아 저장할 곳
	unsigned int Total_Meta_Size;				//위 3개를 총합한 크기

	char Parent_Dir [MAX_PATH];					//부모 디렉토리의 경로를 저장할 변수
	char *Each_Str;								//각 파일/디렉토리 문자열 포인터
	unsigned int FileOrDir_Length;				//파일/디렉토리의 이름 길이
	unsigned int Each_Str_Len;					//각 파일/디렉토리 문자열의 길이

	HANDLE hEachFile;							//각 파일을 기록할 때 쓰이는 핸들
	HANDLE hDxa;								//작업을 마친 후 새 wolf 파일을 기록할 때 쓰이는 핸들


	CString tmpCstr;							//있으면 유용하게 써먹을 수 있다
	wchar_t *List_Buff_Backup;					//*경로를 여는 데 쓰일 백업버퍼
	unsigned int Uni_List_Size;					//*위 백업버퍼의 크기
	wchar_t ***Dir_File_Wstr_Pnt;				//*경로를 지정하는 포인터
		

	DWORD lp_read, lp_write;


	TCHAR dxaname[MAX_PATH];
	memset (dxaname, 0, sizeof(TCHAR) * MAX_PATH);
	PakDlg->GetDlgItemText (IDC_P_EDIT1, dxaname, MAX_PATH);
	//*파일 이름 얻어오기

	if (code == JAP_CODE) { List_Buff = U2J (Wolf_File_List.GetBuffer()); }
	else if (code == KOR_CODE) { List_Buff = U2K (Wolf_File_List.GetBuffer()); }
	List_Size = (unsigned int)strlen(List_Buff);

	List_Buff_Backup = Wolf_File_List.GetBuffer();
	Uni_List_Size = (unsigned int)wcslen(List_Buff_Backup);
	//*List_Buff 변수는 문자열을 긁어와 계산할 때 쓰일 변수고,
	//*List_Buff_Backup은 디렉토리/파일을 열 때 쓰일 변수다
	//*그러므로 List_Buff는 지정된 언어코드에 따라 바꾼다.
	//***0. 리스트 읽어오기


	
	Dir_Idx_Collect_Size = 0;
	for (unsigned int i = 0;i < List_Size-0x3;i++) {
		if (*(unsigned int*)(&List_Buff[i]) == 0x0A0D0A0D) { Dir_Idx_Collect_Size++; }
	}
	//*루트 디렉토리를 포함한 총 디렉토리 수 추산. 개행이 2번 연달아 있는 횟수를 추산하면 된다

	Dir_Str_MetaCnt_Arr = (FOD_IN_DIR*)malloc(sizeof(FOD_IN_DIR)*Dir_Idx_Collect_Size);
	memset (Dir_Str_MetaCnt_Arr, 0, sizeof(FOD_IN_DIR)*Dir_Idx_Collect_Size);
	//*디렉토리가 가진 메타데이터 수를 추산해 저장하는 배열 생성 및 초기화

	Dir_Str_MetaCnt_Arr[0].Dir_Str = List_Buff;
	//*마침 제일 처음에 있는 게 루트 디렉토리므로 이렇게 하면 된다

	Metadata_Collect_Size = 1;
	Dir_Idx_Tmp = 0;
	for (unsigned int i = 0;i < List_Size-4;i++) {
		if (List_Buff[i] == 0x9) { Metadata_Collect_Size++; Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt++; }
		//*탭을 만났을 때. 메타데이터 배열수 뿐만 아니라 각 디렉토리 소유 메타데이터 수도 늘려줌

		else if ((List_Buff[i] == 0xD) && (List_Buff[i+1] == 0xA)) { 
			if ((List_Buff[i+2] == 0xD) && (List_Buff[i+3] == 0xA)) { 
				Dir_Idx_Tmp++; 
				Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Str = &List_Buff[i+4];
			}
			//*개행이 연속 2번일 때 -> 다음 디렉토리로 넘어감을 뜻함. 그러므로 그 시작지점을 문자열로 삼기로 한다
		}
		//*개행을 만났을 때 그 부분을 아예 0으로 만들어 버려야 하지만, 밑의 과정 때문에 바로 그러지는 않는다
	}
	//*루트 디렉토리를 포함한 총 메타데이터 수 추산. 탭의 수를 기준으로 추산하면 되지만 루트 디렉토리는 포함 안되므로 1을 더 더한다
	//*추가로 개행 부분을 0으로 만들어버려서 간단하게 파일/디렉토리에 접근 가능한 문자열을 만들고, 해당 디렉토리가 가진 메타데이터를 늘려야 한다

	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {
		Dir_Str_MetaCnt_Arr[i].FileOrDir_In_Dir_Arr = 
			(char**)malloc(sizeof(char*) * Dir_Str_MetaCnt_Arr[i].Dir_Meta_Cnt);
		memset (Dir_Str_MetaCnt_Arr[i].FileOrDir_In_Dir_Arr, 0, sizeof(char*) * Dir_Str_MetaCnt_Arr[i].Dir_Meta_Cnt);
	}
	//*문자열 포인터 배열 할당 및 초기화
	


	Dir_File_Wstr_Pnt = (wchar_t***)malloc(sizeof(wchar_t**) * Dir_Idx_Collect_Size);
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {
		Dir_File_Wstr_Pnt[i] = (wchar_t**)malloc (sizeof(wchar_t*) * (Dir_Str_MetaCnt_Arr[i].Dir_Meta_Cnt + 1));
	}
	Dir_File_Wstr_Pnt[0][0] = List_Buff_Backup;
	Dir_Idx_Tmp = 0;
	for (unsigned int i = 0;i < Uni_List_Size-4;i++) { 
		if ((List_Buff_Backup[i] == 0xD) && (List_Buff_Backup[i+1] == 0xA) 
			&& (List_Buff_Backup[i+2] == 0xD) && (List_Buff_Backup[i+3] == 0xA)) { 
				Dir_Idx_Tmp++;
				Dir_File_Wstr_Pnt[Dir_Idx_Tmp][0] = &List_Buff_Backup[i+4];
		} 
	}
	//***경로에 접근하기 위한 포인터도 추가 할당, +1은 해당 덩어리에 대응하는 디렉토리를 표시해야 하기 때문



	Dir_Idx_Tmp = FoD_Idx_Tmp = 0;
	for (unsigned int i = 0;i < List_Size-1;i++) {
		if (List_Buff[i] == 0x9) { 
			Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp] = &List_Buff[i+1];
			FoD_Idx_Tmp++;
		}
		//*탭을 만났을 때. 이번에는 각각의 문자열들을 제대로 매칭시켜준다(물론 탭은 넘기고)

		else if ((List_Buff[i] == 0xD) && (List_Buff[i+1] == 0xA)) { 
			List_Buff[i] = List_Buff[i+1] = 0; 
			if ((List_Buff[i+2] == 0xD) && (List_Buff[i+3] == 0xA)) { 
				FoD_Idx_Tmp = 0;Dir_Idx_Tmp++; 
				if ((List_Buff[i+4] == 0xD) && (List_Buff[i+5] == 0xA)) { 
					i--; while ((List_Buff[i] != 0) && (i >= 0)) { i--; }
					if (code == JAP_CODE) { tmpCstr = J2U (&List_Buff[i+1]); }
					else if (code == KOR_CODE) { tmpCstr = K2U (&List_Buff[i+1]); }
					AfxMessageBox (tmpCstr + _T(" : 디렉토리가 비었습니다.\n리스트에서 삭제해주세요."));
					for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
						free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
						free (Dir_File_Wstr_Pnt[jj]);
					}
					free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt); tmpCstr.Empty();
					return;
				}
			}
			//*개행이 연속 2번일 때 -> 다음 디렉토리로 넘어감을 뜻함.
			//*그러므로 [파일/디렉토리] 인덱스를 초기화하고 디렉토리 인덱스를 더한다
		}
		//*이번에야말로 개행 부분을 0으로 만든다
	}
	//*각 디렉토리 소유 [파일/디렉토리] 배열 세팅



	Dir_Idx_Tmp = FoD_Idx_Tmp = 0;
	for (unsigned int i = 0;i < Uni_List_Size-1;i++) {
		if (List_Buff_Backup[i] == 0x9) { 
			Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1] = &List_Buff_Backup[i+1];
			FoD_Idx_Tmp++;
		}
		//*탭을 만났을 때. 이번에는 각각의 문자열들을 제대로 매칭시켜준다(물론 탭은 넘기고)

		else if ((List_Buff_Backup[i] == 0xD) && (List_Buff_Backup[i+1] == 0xA)) { 
			List_Buff_Backup[i] = List_Buff_Backup[i+1] = 0; 
			if ((List_Buff_Backup[i+2] == 0xD) && (List_Buff_Backup[i+3] == 0xA)) 
			{ FoD_Idx_Tmp = 0;Dir_Idx_Tmp++; }
			//*개행이 연속 2번일 때 -> 다음 디렉토리로 넘어감을 뜻함.
			//*그러므로 [파일/디렉토리] 인덱스를 초기화하고 디렉토리 인덱스를 더한다
		}
		//*이번에야말로 개행 부분을 0으로 만든다
	}
	//***접근을 위한 경로 포인터에도 추가로 세팅



	FileDirnames_Raw_Size = 0;
	FilesBuff_Size = 0;
	//*[파일이름+체크섬] 원데이터 크기 및 파일 버퍼 크기 초기화


	List_Buff_Pnt = List_Buff;
	Dir_Idx_Tmp = FoD_Idx_Tmp = 0;
	for (unsigned int i = 0;i < Metadata_Collect_Size;i++) {

		if (i == 0) { FileDirnames_Raw_Size += (sizeof(short) * 2); }
		//*루트 디렉토리를 상징하는데, 길이가 아예 없으므로 4만 더한다

		else {

			if (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt == 0) { 
				FoD_Idx_Tmp = 0; Dir_Idx_Tmp++; continue; 
			}

			while (*List_Buff_Pnt != 0x9) { List_Buff_Pnt++; } List_Buff_Pnt++;
			//*탭 바로 다음곳을 찾기(경로의 시작)

			FileOrDir_Length = (unsigned int)strlen(List_Buff_Pnt) - (Get_Last_SlashPos(List_Buff_Pnt, code)+1);
			FileOrDir_Length += (4-(FileOrDir_Length%4));
			//*마지막 슬래시 너머의 문자열 길이만 얻어내서 4로 정렬 후 더해야 한다

			FileDirnames_Raw_Size += ((sizeof(short) + FileOrDir_Length) * 2);
			//*(길이*2) + 4만큼 더한다

			if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == 1) { 
				hEachFile = CreateFileW (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				Each_File_Size = GetFileSize (hEachFile, NULL);
				FilesBuff_Size += Each_File_Size + (4-(Each_File_Size%4))%4;
				CloseHandle (hEachFile);
				//*파일일 때 파일 크기 더하기
				//*이건 압축하지 않았을 때의 크기이고, 어차피 압축하게 된다 한들 크기면에서 벗어나지 않기에
				//*이러는 게 제일 낫다. 기록할 때는 FilesBuff_Size 만큼만 하면 된다
			}
			else if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == -1) {
				tmpCstr = Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]; 
				AfxMessageBox (tmpCstr + _T(" : 해당 파일/디렉토리가 없습니다."));
				for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
					free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
					free (Dir_File_Wstr_Pnt[jj]);
				} 
				free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
				return;
			}
			else if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == 0) 
			{;}
			//*덤으로 파일 버퍼의 크기도 확정짓는다.(신버전에서는 파일 버퍼 오프셋도 4로 나눠떨어져야 한다)

			FoD_Idx_Tmp++;
			if (FoD_Idx_Tmp >= Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt) 
			{ FoD_Idx_Tmp = 0; Dir_Idx_Tmp++; }
			//*갱신 방향을 잘못 잡을 뻔했음

		}
		//*다른 파일 or 디렉토리

	}
	//*[파일명+체크섬] 길이 계산 시, 파일 이름 길이는 4로 올려 정렬해 계산한다
	//*여기서 덤으로 파일 버퍼에 필요한 길이도 계산한다

	Files_Buff = (unsigned char*)malloc(sizeof(char)*FilesBuff_Size);
	if (Files_Buff == NULL) {
#ifdef _WIN64
		AfxMessageBox (_T(" : 용량이 지나치게 커서 합성할 수 없습니다."));
#else
		AfxMessageBox (_T(" : 총합 용량이 너무 큽니다. 64비트로 돌리십시오."));
#endif
		return;
	}
	memset (Files_Buff, 0, FilesBuff_Size);
	//*파일 버퍼 활성화 및 초기화

	Total_Meta_Size = FileDirnames_Raw_Size + 
		sizeof(FILE_BIG_METADATA)*Metadata_Collect_Size + 
		sizeof(DIR_BIG_IDX)*Dir_Idx_Collect_Size;
	Total_Meta_Buff = (unsigned char*)malloc(sizeof(char)*Total_Meta_Size);
	memset (Total_Meta_Buff, 0, Total_Meta_Size);
	//*총합 메타데이터 공간 계산 후 할당 및 초기화

	FileDirnames_Raw = Total_Meta_Buff;
	Metadata_Collect = (FILE_BIG_METADATA*)(Total_Meta_Buff + FileDirnames_Raw_Size);
	Dir_Idx_Collect = (DIR_BIG_IDX*)(Total_Meta_Buff + FileDirnames_Raw_Size + sizeof(FILE_BIG_METADATA)*Metadata_Collect_Size);
	//*각 위치에 포인터 세팅

	//***1. 루트 디렉토리를 포함해 총 [파일이름+체크섬] 배열 원소 수, 파일 버퍼 크기, 디렉토리 인덱스 수, 총 메타데이터 수 추산





	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {

		Dir_Idx_Collect[i].mt_number = (__int64)(Dir_Str_MetaCnt_Arr[i].Dir_Meta_Cnt);
		//*각 디렉토리가 소유하는 메타데이터 수를 미리 세놓았으므로 대입해 준다

		if (i == 0) {
			Dir_Idx_Collect[i].dirmt_ofs = (__int64)0x0;
			//*루트 디렉토리 메타데이터 맨 처음에 있으므로 당연히 0을 넣어야 한다
			Dir_Idx_Collect[i].parent_dir = (__int64)-1;
			//*루트 디렉토리 부모 디렉토리는 없으므로 -1
			Dir_Idx_Collect[i].mt_offset = (__int64)(sizeof(FILE_BIG_METADATA));
			//*루트 디렉토리가 소유한 메타데이터는 루트 디렉토리 메타데이터 다음에 있으므로 그 크기만큼만 더해주면 된다
		}
		//*루트 디렉토리 세팅

		else {
			memset (Parent_Dir, 0, MAX_PATH);
			memcpy (Parent_Dir, 
				Dir_Str_MetaCnt_Arr[i].Dir_Str, 
				Get_Last_SlashPos (Dir_Str_MetaCnt_Arr[i].Dir_Str, code));
			//*마지막 슬래시 이전까지만 문자열을 복사하면 그게 부모 디렉토리가 된다

			Dir_Idx_Collect[i].parent_dir = (__int64)-1;
			for (Dir_Idx_Tmp = 0 ; Dir_Idx_Tmp < i ; Dir_Idx_Tmp++) {
				if (strcmp (Parent_Dir, Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Str) == 0) {
					Dir_Idx_Collect[i].parent_dir = (__int64)(sizeof(DIR_BIG_IDX) * Dir_Idx_Tmp);
					break;
				}
			}
			if (Dir_Idx_Tmp == i) {
				tmpCstr = Dir_File_Wstr_Pnt[i][0];
				AfxMessageBox (tmpCstr + _T(" : 부모 디렉토리가 없습니다.\n리스트가 깨진 것 같습니다."));
				for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
					free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
					free (Dir_File_Wstr_Pnt[jj]);
				}
				free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
				free (Files_Buff); free (Total_Meta_Buff);
				return;
				//*만일 여기에 이르러서까지 부모 디렉토리가 없다고 나오면 분명 잘못된 것이다
			}
			//*따라서 그것을 앞서 선언한 Dir_Str_MetaCnt_Arr로부터 문자열 포인터를 얻어 비교하면 된다(부모 디렉토리 세팅)
			//*Dir_Idx_Tmp는 현재 세팅 중인 디렉토리의 부모 디렉토리 인덱스를 가지게 된다

			Dir_Idx_Collect[i].mt_offset = (__int64)(sizeof(FILE_BIG_METADATA));
			//*루트 디렉토리 메타데이터 오차 보정
			for (unsigned int j = 0;j < i;j++) {
				Dir_Idx_Collect[i].mt_offset += (__int64)(sizeof(FILE_BIG_METADATA) * Dir_Str_MetaCnt_Arr[j].Dir_Meta_Cnt);
			}
			//*그냥 [이전 디렉토리까지의 총 메타데이터 수] + [루트 디렉토리의 메타데이터] 잘 더하면 될 것이다
			//*디렉토리 소유 메타데이터 오프셋 세팅

			for (FoD_Idx_Tmp = 0 ; FoD_Idx_Tmp < Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt ; FoD_Idx_Tmp++) {
				if (strcmp (Dir_Str_MetaCnt_Arr[i].Dir_Str, 
					Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp]) == 0) {
					break;
				}
				//*현재 디렉토리 문자열과 부모 디렉토리 내 [파일/디렉토리] 문자열들과 비교
			}
			if (FoD_Idx_Tmp == Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt) {
				tmpCstr = Dir_File_Wstr_Pnt[Dir_Idx_Tmp][0];
				tmpCstr += _T("\n");
				tmpCstr += Dir_File_Wstr_Pnt[i][0];
				tmpCstr += _T(" 디렉토리 내에 해당 디렉토리가 없습니다.\n리스트가 깨진 것 같습니다.");
				AfxMessageBox (tmpCstr);
				for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
					free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr);
					free (Dir_File_Wstr_Pnt[jj]);
				}
				free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
				free (Files_Buff); free (Total_Meta_Buff);
				return;
				//*만일 얻어내지 못했다면 에러처리
			}
			//*부모 디렉토리의 메타데이터 오프셋은 알고 있으므로 (루트 디렉토리는 0x0, 이하 디렉토리들은 위에서 미리 구한다)
			//*부모 디렉토리로부터 순차적으로 내려가서 어디서 문자열이 '같은지' 파악하면 된다
			Dir_Idx_Collect[i].dirmt_ofs = (__int64)(Dir_Idx_Collect[Dir_Idx_Tmp].mt_offset
				 + ((sizeof(FILE_BIG_METADATA) * FoD_Idx_Tmp)));
			//*그러면 (부모 디렉토리의 오프셋) + (메타데이터 원소크기*내려간 수)가 될 것이다
			//*디렉토리 오프셋(in 메타데이터) 세팅
			//*FoD_Idx_Tmp는 부모 디렉토리 내에 해당 디렉토리가 있는 인덱스를 가리킨다
		}
		//*다른 디렉토리 세팅
	}
	//***2. [디렉토리 인덱스] 배열 세팅. 어차피 메타데이터가 순차적으로 세팅되어 있으므로 순서대로만 잘 하면 된다





	Dir_Idx_Tmp = (sizeof(short)*2);
	if (Set_Big_FileOrDirName_Data (FileDirnames_Raw, &Dir_Idx_Tmp, 0, Dir_Str_MetaCnt_Arr, 
		Dir_Idx_Collect_Size, Metadata_Collect, code, Dir_File_Wstr_Pnt) == -1)
	{
		tmpCstr = dxaname; AfxMessageBox (tmpCstr + _T(" : 메타데이터 구성을 실패했습니다.\n"));
		for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
			free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
			free (Dir_File_Wstr_Pnt[jj]);
		}
		free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt); 
		free (Files_Buff); free (Total_Meta_Buff);
		return;
		//*-1이 반환됐다면 에러처리
	}
	//***3. [파일명+체크섬] 배열 세팅. 이 때 Dir_Str_MetaCnt_Arr를 참조한다
	//***(인자 : FileDirnames_Raw, FileDirnames_Collect 인덱스(포인터), Dir_Str_MetaCnt_Arr, 
	//***디렉토리 인덱스의 인덱스(Dir_Idx_Tmp역할), Metadata_Collect), 코드값 -> int값 반환(-1이 에러)
	//***어차피 루프 디렉토리는 그냥 0이니까 미리 4바이트를 넘기고 시작한다






	Dir_Idx_Tmp = FoD_Idx_Tmp = 0;
	Offset_Acumm = 0;
	for (unsigned int i = 0;i < Metadata_Collect_Size;i++) {

		PakDlg->m_Pack_Progress.SetPos (PROGRESSVALUE(i, Metadata_Collect_Size));
		//*프로그레스 바 세팅

		if (i == 0) {
			Metadata_Collect[i].fltok_start = (__int64)0x0;
			Metadata_Collect[i].attr = (__int64)(DIR);
			Metadata_Collect[i].generated = Metadata_Collect[i].last_modified = 
				Metadata_Collect[i].last_modified = (__int64)0x0;
			Metadata_Collect[i].offset = Metadata_Collect[i].UncompSize = (__int64)0x0;
			Metadata_Collect[i].CompSize = (__int64)-1;
		}
		//*루트 디렉토리일 때. 디렉토리고, 생성, 마지막 수정, 마지막 접속 같은 건 의미가 없으므로 0 처리한다
		//*물론 오프셋, 비압축사이즈, 압축사이즈도 의미가 없다

		else {

			if (Is_File_Or_Dir(Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == 0) {
				Metadata_Collect[i].attr = DIR;		//*속성 : 디렉토리

				hEachFile = CreateFileW (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				GetFileTime(hEachFile, 
					(LPFILETIME)&(Metadata_Collect[i].generated), 
					(LPFILETIME)&(Metadata_Collect[i].last_accessed),
					(LPFILETIME)&(Metadata_Collect[i].last_modified));
				CloseHandle (hEachFile);
				//*생성, 접속 시간, 수정 시간 반영

				Metadata_Collect[i].UncompSize = (__int64)0x0;
				Metadata_Collect[i].CompSize = (__int64)-1;
				//*비압축크기 0, 압축크기 -1로 처리

				Metadata_Collect[i].offset = (__int64)0x0;
				for (unsigned int j = 0;j < Dir_Idx_Collect_Size;j++) {
					if (strcmp(Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp], 
						Dir_Str_MetaCnt_Arr[j].Dir_Str) == 0) {
							Metadata_Collect[i].offset = (__int64)(sizeof(DIR_BIG_IDX) * j); break;
					}
				}
				//*오프셋의 값으로 디렉토리 인덱스 내의 오프셋을 반영해야 한다
			}
			//*디렉토리일 때. 생성, 수정, 접속시간, 비압축사이즈, 압축사이즈의 의미가 없다

			else if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]) == 1) {


				if (!CanBeFile (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp], code)) {
					int ll = Get_Last_SlashPos (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp], code);
					if (code == JAP_CODE) { tmpCstr = J2U (&Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp][ll + 1]); }
					else if (code == KOR_CODE) { tmpCstr = K2U (&Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp][ll + 1]); }
					AfxMessageBox (tmpCstr + _T(" : 언어코드가 달라 파일을 합성할 수 없습니다.\n언어코드를 바꿔보십시오.")); tmpCstr.Empty();
					for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
						free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
						free (Dir_File_Wstr_Pnt[jj]);
					}
					free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
					free (Files_Buff); free (Total_Meta_Buff);
					return;
				}
				//***파일 이름이 될 문자열 체크 : 물음표가 끼어 있으면 나중에 출력을 할 수 없으므로 에러처리한다.
				//***체크 조건 : 마지막 슬래시까지 찾아가다가 찾기 전에 물음표 나오면 불가능


				tmpCstr = _T("Packing : ");
				PakDlg->SetDlgItemText (IDC_P_TEXT, tmpCstr + Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1]);
				//*출력 문자열 갱신

				
				Metadata_Collect[i].attr = FILE_DB;	//*속성 : 파일

				hEachFile = CreateFileW (Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				Each_File_Size = GetFileSize (hEachFile, NULL);
				Xor_Ofs = (Each_File_Size % KEY_SIZE);

				if (Each_File_Size != 0) { 
					Each_File_Buff = (unsigned char*)malloc(sizeof(char) * Each_File_Size * 3); 
					memset (Each_File_Buff, 0, Each_File_Size * 3);
					ReadFile (hEachFile, Each_File_Buff, Each_File_Size, &lp_read, NULL);
				}
				else { Each_File_Buff = NULL; }
				//*파일크기 0일 때 설정

				Metadata_Collect[i].UncompSize = (__int64)Each_File_Size;
				//*파일 핸들 열고 읽어오기, 비압축 크기 반영, 보정치 확정

				GetFileTime(hEachFile, 
					(LPFILETIME)&(Metadata_Collect[i].generated), 
					(LPFILETIME)&(Metadata_Collect[i].last_accessed),
					(LPFILETIME)&(Metadata_Collect[i].last_modified));
				CloseHandle (hEachFile);
				//*생성, 접속 시간, 수정 시간 반영

				//***파일 크기가 0일 때도 필터를 걸어서 처리해 준다

				Each_Str = Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp];
				Each_Str_Len = (unsigned int)strlen(Each_Str);
				if ((strcmp(Each_Str + Each_Str_Len - (unsigned int)strlen(".common"), ".common") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - (unsigned int)strlen(".dat"), ".dat") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - (unsigned int)strlen(".project"), ".project") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - (unsigned int)strlen(".xxxxx"), ".xxxxx") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - (unsigned int)strlen(".mps"), ".mps") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - (unsigned int)strlen(".mid"), ".mid") == 0)
					|| (strcmp(Each_Str + Each_Str_Len - (unsigned int)strlen(".txt"), ".txt") == 0)) {

					if (strcmp(Each_Str + Each_Str_Len - (unsigned int)strlen("Game.dat"), "Game.dat") == 0) {
						Metadata_Collect[i].CompSize = (__int64)-1;
						//*예외로 Game.dat만은 압축저장하지 않는다

//						if (Each_File_Buff[0x1F] == 0x1) { Each_File_Buff[0x1F] = 0x2; }
//						Each_File_Buff[0x1F] = 0x2;
//						//*중간의 버퍼 중에 0x1F 번째 바이트가 0x1로 되어 있다면 0x2로 바꿔야 한글이 나온다

						if (type == DXA_330) {
							for (unsigned int j = 0;j < Each_File_Size;j++) {
								Each_File_Buff[j] ^= WOLF_KEY3[(j+Xor_Ofs)%KEY_SIZE];
							}
						}
						else if (type == DXA_THMK_64) {
							for (unsigned int j = 0;j < Each_File_Size;j++) {
								Each_File_Buff[j] ^= THMK2_KEY[(j+Xor_Ofs)%KEY_SIZE];
							}
						}
						else {
							for (unsigned int j = 0;j < Each_File_Size;j++) {
								Each_File_Buff[j] ^= WOLF_KEY3[(j+Xor_Ofs)%KEY_SIZE];
							}
						}
						//*위치 보정하고 xor하기
					}
					else {

						if (Each_File_Size != 0) { 
							Encode (Each_File_Buff, Each_File_Size);
							Metadata_Collect[i].CompSize = (__int64)(*(unsigned int*)(&Each_File_Buff[4]));
							Each_File_Size = (unsigned int)Metadata_Collect[i].CompSize;
							//*압축 완료된 버퍼 내에서 압축된 크기 확인

							if (type == DXA_330) {
								for (unsigned int j = 0;j < (unsigned int)Metadata_Collect[i].CompSize;j++) {
									Each_File_Buff[j] ^= WOLF_KEY3[(j+Xor_Ofs)%KEY_SIZE];
								}
							}
							else if (type == DXA_THMK_64) {
								for (unsigned int j = 0;j < (unsigned int)Metadata_Collect[i].CompSize;j++) {
									Each_File_Buff[j] ^= THMK2_KEY[(j+Xor_Ofs)%KEY_SIZE];
								}
							}
							else {
								for (unsigned int j = 0;j < (unsigned int)Metadata_Collect[i].CompSize;j++) {
									Each_File_Buff[j] ^= WOLF_KEY3[(j+Xor_Ofs)%KEY_SIZE];
								}
							}
							//*위치 보정하고 xor하기
						}
						//*파일 크기가 0보다 클 때만 수행한다
						else { Metadata_Collect[i].CompSize = (__int64)(-1); }
						//*파일 크기가 0이라면 압축한 크기가 없으므로 -1로 잡아도 될까?

					}
				}
				else {
					Metadata_Collect[i].CompSize = (__int64)-1;
					//*압축할 파일이 아니라면 압축크기를 -1로 잡아버린다
					if (type == DXA_330) {
						for (unsigned int j = 0;j < Each_File_Size;j++) {
							Each_File_Buff[j] ^= WOLF_KEY3[(j+Xor_Ofs)%KEY_SIZE];
						}
					}
					if (type == DXA_THMK_64) {
						for (unsigned int j = 0;j < Each_File_Size;j++) {
							Each_File_Buff[j] ^= THMK2_KEY[(j+Xor_Ofs)%KEY_SIZE];
						}
					}
					else {
						for (unsigned int j = 0;j < Each_File_Size;j++) {
							Each_File_Buff[j] ^= WOLF_KEY3[(j+Xor_Ofs)%KEY_SIZE];
						}
					}
					//*위치 보정하고 xor하기
				}
				//*파일 종류에 따라 압축, 비압축을 설정한다
				//*만일 확장자가 .dat, .project, .xxxxx, .mps, .mid가 들어가있으면 이것들을 압축해 저장해야 한다

				Metadata_Collect[i].offset = Offset_Acumm;
				if (Each_File_Size != 0) { memcpy (Files_Buff + Offset_Acumm, Each_File_Buff, Each_File_Size); }
				Offset_Acumm += (Each_File_Size + (4 - (Each_File_Size)%4)%4);
				//*오프셋 저장, 파일 버퍼에 복사, 오프셋 누적시키기

				if (Each_File_Size != 0) { free (Each_File_Buff); }
				//*파일 버퍼 해제
			}
			//*파일일 때. 어떤 파일들은 압축할 필요가 있다.

			else {
				tmpCstr = Dir_File_Wstr_Pnt[Dir_Idx_Tmp][FoD_Idx_Tmp + 1];
				if (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].FileOrDir_In_Dir_Arr[FoD_Idx_Tmp] == NULL) {
					tmpCstr += _T(" : 해당 디렉토리는 다른 파일 혹은 디렉토리를 소유하지 않습니다.\n이 디렉토리를 리스트에서 제거한 후 다시 시도하십시오.");
				}
				else {
					tmpCstr += _T(" : 해당 문자열은 [파일/디렉토리]로서 존재하지 않습니다.");
				}
				AfxMessageBox (tmpCstr);

				for (unsigned int jj = 0;jj < Dir_Idx_Collect_Size;jj++) { 
					free (Dir_Str_MetaCnt_Arr[jj].FileOrDir_In_Dir_Arr); 
					free (Dir_File_Wstr_Pnt[jj]);
				}
				free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
				free (Files_Buff); free (Total_Meta_Buff);
				return;
			}
			//*해당 문자열이 디렉토리나 파일의 경로가 아닐 때 -> 에러

			FoD_Idx_Tmp++;
			if (FoD_Idx_Tmp >= (Dir_Str_MetaCnt_Arr[Dir_Idx_Tmp].Dir_Meta_Cnt)) {
				FoD_Idx_Tmp = 0; Dir_Idx_Tmp++;
			}
			//*만일 한 디렉토리 내의 메타데이터 및 파일을 전부 반영했다면,
			//*다음 디렉토리로 넘어가기 위해 적절하게 초기화한다

			//*[디렉토리 인덱스] 순으로 맞추어 정리한다
			//*메타데이터는 순차적으로 맞추어 정리해 나가기에 위와 같이 단순하게 해도 문제없다
		}
		//*이외의 다른 [디렉토리/파일]일 때. 각 디렉토리의 메타데이터 소유 수를 참조하여 촤라락 정리한다
	}
	//*디렉토리(0x10) 아니면 파일(0x20)으로 일괄 정리하도록 한다. 순서대로 적용 가능하니 편할듯.
	//*fltok_start는 위 과정에서 세팅되므로 굳이 건들지 않아도 된다
	//*위 작업을 다 마치면 Offset_Acumm이 최종 파일 버퍼 크기가 된다

	if (type == DXA_330) {
		for (unsigned int j = 0;j < Total_Meta_Size;j++) {
			Total_Meta_Buff[j] ^= WOLF_KEY3[(j)%KEY_SIZE];
		}
	}
	if (type == DXA_THMK_64) {
		for (unsigned int j = 0;j < Total_Meta_Size;j++) {
			Total_Meta_Buff[j] ^= THMK2_KEY[(j)%KEY_SIZE];
		}
	}
	else {
		for (unsigned int j = 0;j < Total_Meta_Size;j++) {
			Total_Meta_Buff[j] ^= WOLF_KEY3[(j)%KEY_SIZE];
		}
	}
	//*이후 위치 보정 xor

	//***4. [메타데이터] 배열 및 파일 버퍼 세팅, 이때 파일버퍼도 같이 세팅한다




	memset (&Dxa_Head, 0, sizeof(WOLF_BIG_HEADER));
	if (type == DXA_330) { Dxa_Head.magic = WOLF_MAGIC_64; }
	else if (type == DXA_THMK_64) { Dxa_Head.magic = DXA_MAGIC_64; }
	else { Dxa_Head.magic = WOLF_MAGIC_64; }
	Dxa_Head.total_toklen = Total_Meta_Size;
	Dxa_Head.header_size = (__int64)(sizeof(WOLF_BIG_HEADER));
	Dxa_Head.totalmt_ofs = (__int64)(Offset_Acumm + sizeof(WOLF_BIG_HEADER));
	Dxa_Head.filemt_ofs = (__int64)(FileDirnames_Raw_Size);
	Dxa_Head.dir_hd_ofs = (__int64)(Total_Meta_Size - (sizeof(DIR_BIG_IDX)*Dir_Idx_Collect_Size));
	Dxa_Head.end = (__int64)(HEAD_END2);
	unsigned char *pt = (unsigned char*)(&Dxa_Head);
	if (type == DXA_330) {
		for (unsigned int i = 0;i < sizeof(WOLF_BIG_HEADER);i++) { pt[i] ^= WOLF_KEY3[(i)%KEY_SIZE]; }
	}
	if (type == DXA_THMK_64) {
		for (unsigned int i = 0;i < sizeof(WOLF_BIG_HEADER);i++) { pt[i] ^= THMK2_KEY[(i)%KEY_SIZE]; }
	}
	else {
		for (unsigned int i = 0;i < sizeof(WOLF_BIG_HEADER);i++) { pt[i] ^= WOLF_KEY3[(i)%KEY_SIZE]; }
	}
	//***5. 헤더를 하나 잡고 올바른 값을 기술하고 xor 한다




	hDxa = CreateFileW (dxaname, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile (hDxa, &Dxa_Head, sizeof(WOLF_BIG_HEADER), &lp_write, NULL);
	WriteFile (hDxa, Files_Buff, (unsigned int)Offset_Acumm, &lp_write, NULL);
	WriteFile (hDxa, Total_Meta_Buff, Total_Meta_Size, &lp_write, NULL);
	
	PakDlg->m_Pack_Progress.SetPos (100);
	PakDlg->SetDlgItemText (IDC_P_TEXT, _T("Packing Finished"));
//	printf("\n합성 완료\n");
	//***6. 헤더, 파일공간, 총합 메타데이터 순으로 파일을 기록한다




	CloseHandle (hDxa);
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) { 
		free (Dir_Str_MetaCnt_Arr[i].FileOrDir_In_Dir_Arr); 
		free (Dir_File_Wstr_Pnt[i]);
	}
	free (Dir_Str_MetaCnt_Arr); free (Dir_File_Wstr_Pnt);
	free (Files_Buff); free (Total_Meta_Buff);
	//***7. 전부 해제
}





int Set_FileOrDirName_Data (unsigned char *FileDirnames_Raw, unsigned int *FileDirnames_Pnt_Addr,
	unsigned int Dir_Idx, FOD_IN_DIR *Dir_Str_MetaCnt_Arr, unsigned int Dir_Idx_Size,
	FILE_METADATA *Metadata_Collect, unsigned int code, wchar_t ***Dir_File_Wstr_Pnt)
{
	unsigned int Def_Len, Set_Len, SlashPos, Meta_Idx, tmp, Ret;
	unsigned short CheckSum;

	for (unsigned int i = 0;i < Dir_Str_MetaCnt_Arr[Dir_Idx].Dir_Meta_Cnt;i++) {

		SlashPos = Get_Last_SlashPos(Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i], code);
		Def_Len = (unsigned int)strlen(Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i] + SlashPos + 1);
		Set_Len = Def_Len + (4 - (Def_Len%4));
		//*더해야 할 길이 확정

		Meta_Idx = 1;
		for (unsigned int j = 0;j < Dir_Idx;j++) { Meta_Idx += Dir_Str_MetaCnt_Arr[j].Dir_Meta_Cnt; }
		Meta_Idx += i; tmp = (*FileDirnames_Pnt_Addr);
		Metadata_Collect[Meta_Idx].fltok_start = (tmp);
		//*해당 메타데이터에 미리 위치 반영

		*(unsigned short*)(FileDirnames_Raw + (*FileDirnames_Pnt_Addr)) = (Set_Len/4);
		(*FileDirnames_Pnt_Addr) += sizeof(short);
		//*(이름길이/4) 반영

		Converting_Big ((char*)(FileDirnames_Raw + (*FileDirnames_Pnt_Addr) + sizeof(short)), 
			(Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i] + SlashPos + 1), Def_Len, code);
		//*대문자열 변환 및 반영

		CheckSum = 0;
		for (unsigned int j = 0;j < Def_Len;j++) { 
			CheckSum += *(FileDirnames_Raw + (*FileDirnames_Pnt_Addr) + sizeof(short) + j);
		}
		*(unsigned short*)(FileDirnames_Raw + (*FileDirnames_Pnt_Addr)) = CheckSum;
		(*FileDirnames_Pnt_Addr) += (sizeof(short) + Set_Len);
		//*체크섬 반영

		memcpy (FileDirnames_Raw + (*FileDirnames_Pnt_Addr), 
			Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i] + SlashPos + 1, Def_Len);
		(*FileDirnames_Pnt_Addr) += Set_Len;
		//*원본문자열 반영
		
		if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx][i + 1]) == 0) {
			tmp = Dir_Idx_Size;
			for (unsigned int j = 0;j < Dir_Idx_Size;j++) {
				if (strcmp (Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i], 
					Dir_Str_MetaCnt_Arr[j].Dir_Str) == 0) { tmp = j; break; }
			}
			if (tmp == Dir_Idx_Size) { return -1; }
			//*해당 디렉토리 문자열과 일치하는 디렉토리 문자열 찾기. 없으면 -1 반환
			
			Ret = Set_FileOrDirName_Data (FileDirnames_Raw, FileDirnames_Pnt_Addr, tmp, Dir_Str_MetaCnt_Arr,
				Dir_Idx_Size, Metadata_Collect, code, Dir_File_Wstr_Pnt);
			if (Ret == -1) { return Ret; }
			//*재귀함수 호출. -1이라는 건 뭔가 잘못됐다는 증거이므로 그대로 리턴한다
		}
		//*현재 문자열이 디렉토리라면, 해당 디렉토리 인덱스를 찾아 재귀호출해야 한다
		else if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx][i + 1]) == 1) { ; }
		//*파일이면 그냥그냥 진행하면 된다
		else { return -1; }
		//*이도 저도 아니면 에러다
	}
	return 0;
}
//***해당 문자열에 대응되는 메타데이터 위치는 (1 + Dir_Idx_Tmp까지 모든 소유 수 + Fod)로 다시 계산하면 됨


int Set_Big_FileOrDirName_Data (unsigned char *FileDirnames_Raw, unsigned int *FileDirnames_Pnt_Addr,
	unsigned int Dir_Idx, FOD_IN_DIR *Dir_Str_MetaCnt_Arr, unsigned int Dir_Idx_Size,
	FILE_BIG_METADATA *Metadata_Collect, unsigned int code, wchar_t ***Dir_File_Wstr_Pnt)
{
	unsigned int Def_Len, Set_Len, SlashPos, Meta_Idx, tmp, Ret;
	unsigned short CheckSum;

	for (unsigned int i = 0;i < Dir_Str_MetaCnt_Arr[Dir_Idx].Dir_Meta_Cnt;i++) {

		SlashPos = Get_Last_SlashPos(Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i], code);
		Def_Len = (unsigned int)strlen(Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i] + SlashPos + 1);
		Set_Len = Def_Len + (4 - (Def_Len%4));
		//*더해야 할 길이 확정

		Meta_Idx = 1;
		for (unsigned int j = 0;j < Dir_Idx;j++) { Meta_Idx += Dir_Str_MetaCnt_Arr[j].Dir_Meta_Cnt; }
		Meta_Idx += i; tmp = (*FileDirnames_Pnt_Addr);
		Metadata_Collect[Meta_Idx].fltok_start = (__int64)(tmp);
		//*해당 메타데이터에 미리 위치 반영

		*(unsigned short*)(FileDirnames_Raw + (*FileDirnames_Pnt_Addr)) = (Set_Len/4);
		(*FileDirnames_Pnt_Addr) += sizeof(short);
		//*(이름길이/4) 반영

		Converting_Big ((char*)(FileDirnames_Raw + (*FileDirnames_Pnt_Addr) + sizeof(short)), 
			(Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i] + SlashPos + 1), Def_Len, code);
		//*대문자열 변환 및 반영

		CheckSum = 0;
		for (unsigned int j = 0;j < Def_Len;j++) { 
			CheckSum += *(FileDirnames_Raw + (*FileDirnames_Pnt_Addr) + sizeof(short) + j);
		}
		*(unsigned short*)(FileDirnames_Raw + (*FileDirnames_Pnt_Addr)) = CheckSum;
		(*FileDirnames_Pnt_Addr) += (sizeof(short) + Set_Len);
		//*체크섬 반영

		memcpy (FileDirnames_Raw + (*FileDirnames_Pnt_Addr), 
			Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i] + SlashPos + 1, Def_Len);
		(*FileDirnames_Pnt_Addr) += Set_Len;
		//*원본문자열 반영

		if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx][i + 1]) == 0) {
			tmp = Dir_Idx_Size;
			for (unsigned int j = 0;j < Dir_Idx_Size;j++) {
				if (strcmp (Dir_Str_MetaCnt_Arr[Dir_Idx].FileOrDir_In_Dir_Arr[i], 
					Dir_Str_MetaCnt_Arr[j].Dir_Str) == 0) { tmp = j; break; }
			}
			if (tmp == Dir_Idx_Size) { return -1; }
			//*해당 디렉토리 문자열과 일치하는 디렉토리 문자열 찾기. 없으면 -1 반환
			
			Ret = Set_Big_FileOrDirName_Data (FileDirnames_Raw, FileDirnames_Pnt_Addr, tmp, Dir_Str_MetaCnt_Arr,
				Dir_Idx_Size, Metadata_Collect, code, Dir_File_Wstr_Pnt);
			if (Ret == -1) { return Ret; }
			//*재귀함수 호출. -1이라는 건 뭔가 잘못됐다는 증거이므로 그대로 리턴한다
		}
		//*현재 문자열이 디렉토리라면, 해당 디렉토리 인덱스를 찾아 재귀호출해야 한다
		else if (Is_File_Or_Dir (Dir_File_Wstr_Pnt[Dir_Idx][i + 1]) == 1) { ; }
		//*파일이면 그냥그냥 진행하면 된다
		else { return -1; }
		//*이도 저도 아니면 에러다
	}
	return 0;
}
//***해당 문자열에 대응되는 메타데이터 위치는 (1 + Dir_Idx_Tmp까지 모든 소유 수 + Fod)로 다시 계산하면 됨(64bit)