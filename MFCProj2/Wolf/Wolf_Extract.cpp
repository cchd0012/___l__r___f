#include "stdafx.h"

#include "Wolf_Extract.h"


extern CString Wolf_File_ListName;
extern CString Wolf_File_List;
//*리스트 이름과 내용을 기록하는 데 쓰일 CString


void extracting (Extract_Dialog* ExtDlg, unsigned int type, unsigned int code)
{
	HANDLE hDxa;
	unsigned char *Dxa_Buff, *Dxa_Buff_Pnt;
	unsigned int Dxa_Size;						//64비트식이지만 일단은 int형으로 쓴다
	WOLF_HEADER *Dxa_Head;

	unsigned char *Files_Buff;					//파일들을 한데 모아놓은 버퍼
	unsigned int FilesBuff_Size;				//파일 버퍼의 크기

	unsigned char *FileDirnames_Raw;			//파일이름 토큰을 곧바로 변화시킬 수 없으므로. 정보를 읽어들인 다음 파일이름 정보로 변환한다.
	FILESTR_DATA *FileDirnames_Collect;			//위에서 파일이름 토큰을 변화시켜 저장할 곳(루트 디렉토리부터 시작, 루트는 없는 이름 취급)
	unsigned int FileDirnames_Collect_Size;		//파일이름 토큰들을 저장할 때 그 갯수

	FILE_METADATA *Metadata_Collect;			//파일의 메타데이터를 저장할 곳
	unsigned int Metadata_Collect_Size;			//파일이름 토큰들을 저장할 때 그 갯수

	DIR_IDX *Dir_Idx_Collect;					//디렉토리의 정보를 저장할 곳
	unsigned int *ParentDir_Name_Len_Buff;		//각 부모 디렉토리의 길이를 담아놓는 곳
	unsigned int Dir_Idx_Collect_Size;			//디렉토리 정보를 저장할 때 그 갯수

	unsigned int Parent_Dir_Idx;				//[디렉토리 인덱스] 내의 부모 디렉토리의 위치를 가리킨다
	unsigned int Parent_Dir_FileDirnames_Pos;	//부모 디렉토리의 [이름+체크섬] 위치를 가리킨다
	unsigned int Each_Dir_FileDirnames_Pos;		//자신 디렉토리의 [이름+체크섬] 위치를 가리킨다
	unsigned int Each_FileDirnames_Pos;			//자신(파일 or 디렉토리)의 [이름+체크섬] 위치를 가리킨다
	unsigned int FileInDir_Meta_Start_Pos;		//디렉토리가 소유하는 파일들 메타데이터의 시작지점
	unsigned int Ofs_Of_DirMeta;				//디렉토리가 소유한 디렉토리 메타데이터로 인한 오차 보정 변수

	//*일단 __int64로 해야 맞는 것 같긴 한데, 아무래도 어긋날 우려가 있어 unsigned int로 통일한다

	char Root_Dir [MAX_PATH];					//루트 디렉토리의 경로를 저장할 변수
	char Upper_Dir [MAX_PATH];					//상위 디렉토리의 경로를 저장할 변수
	char FileOrDir_In_Dir [MAX_PATH];			//디렉토리 밑에 속한 파일/디렉토리의 경로를 저장할 변수

	char List_String [MAX_PATH];				//리스트에 로그를 저장하기 위해 쓰이는 버퍼

	unsigned char *Each_File_Buff;				//각 파일을 가리키게 될 버퍼
	unsigned int Each_File_Size;				//각 파일의 크기

	HANDLE hEachFile;							//각 파일을 기록할 때 쓰이는 핸들
		
	DWORD lp_read, lp_write;

	wchar_t *tmpwstr;							//있으면 자주 써먹기 좋음



	hDxa = CreateFileW(ExtDlg->Wolf_Full_Path, 
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hDxa == INVALID_HANDLE_VALUE){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 dxa 파일을 열지 못했습니다."));
		return;
	}
	//*dxa 파일 열기

	Dxa_Size = GetFileSize(hDxa, NULL);
	if(Dxa_Size <= sizeof(WOLF_HEADER)){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 dxa 파일이 아닙니다."));
		CloseHandle(hDxa);return;
	}//*dxa 파일 크기 조사. 최소한 헤더보다는 커야 한다

	Dxa_Buff = (unsigned char*)malloc(sizeof(char) * Dxa_Size);

	Dxa_Buff_Pnt = Dxa_Buff;
	if (ReadFile (hDxa, Dxa_Buff, (DWORD)Dxa_Size, &lp_read, NULL) == 0) {
#ifdef WIN64
		AfxMessageBox (_T(" : 용량이 지나치게 커서 분해할 수 없습니다."));
#else
		AfxMessageBox (_T(" : 해당 파일은 용량이 너무 큽니다. 64비트로 돌리십시오."));
#endif
		return;
	}
	CloseHandle (hDxa);
	for (unsigned int i = 0;i < Dxa_Size;i++) { 
		if (type == DXA_220) { Dxa_Buff[i] ^= WOLF_KEY2[i%KEY_SIZE]; }
		else if (type == DXA_THMK) { Dxa_Buff[i] ^= THMK2_KEY[i%KEY_SIZE]; }
		else { Dxa_Buff[i] ^= WOLF_KEY[i%KEY_SIZE]; }
	}
	//*전부 읽어들인 후 핸들 닫고 전부 xor 까기
	//*그런데 여기서 할당할 때 32비트 프로그램이라 64비트인 __int64와 범위차이가 나는데,
	//*일단 이건 넘기다시피 하고 생각해야겠음

	Dxa_Head = (WOLF_HEADER*)Dxa_Buff;
	if ((Dxa_Head->magic != WOLF_MAGIC) && (Dxa_Head->magic != DXA_MAGIC)) {
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 데이터 파일이 아닙니다."));
		free (Dxa_Buff); return;
	}
	//*헤더 포인터를 만들어 맨 앞쪽을 가리키게 한 후 매직값 조사

	if(Dxa_Size < (Dxa_Head->totalmt_ofs + Dxa_Head->total_toklen)){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 dxa 파일이 아닙니다."));
		free (Dxa_Buff); return;
	}
	//*헤더의 크기 조건 조사

	if((Dxa_Head->total_toklen <= Dxa_Head->filemt_ofs) || (Dxa_Head->total_toklen <= Dxa_Head->dir_hd_ofs)){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 dxa 파일이 아닙니다."));
		free (Dxa_Buff); return;
	}
	//*각 메타데이터들간의 오프셋 관계 조사

	if (*(unsigned int*)(Dxa_Buff + (Dxa_Head->totalmt_ofs)) != 0x00000000) {
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 dxa 파일이 아닙니다."));
		free (Dxa_Buff); return;
		//*이래도 0이 아니라면 에러처리한다
	}
	//*메타데이터 부분 xor 까두기(리스트 기록용)
	//***0. 분해 시작 전의 선제 조건 검사(메타데이터 시작은 무조건 루트부터 시작하기에 0이어야 한다)

	FilesBuff_Size = (Dxa_Head->totalmt_ofs) - (Dxa_Head->header_size);
	Files_Buff = Dxa_Buff+(Dxa_Head->header_size);
	//*[파일 버퍼] 크기 파악 및 복사(xor은 파일 기록 시 함께 한다)
	
	FileDirnames_Raw = (unsigned char*)malloc(sizeof(char)*(Dxa_Head->filemt_ofs));
	memcpy (FileDirnames_Raw, Dxa_Buff+(Dxa_Head->totalmt_ofs), (Dxa_Head->filemt_ofs));
	FileDirnames_Collect_Size = Get_File_Count(FileDirnames_Raw, (Dxa_Head->filemt_ofs));
	if(FileDirnames_Collect_Size == 0xFFFFFFFF){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 파일 이름 토큰이 깨졌습니다."));
		free (FileDirnames_Raw); free (Files_Buff); free (Dxa_Buff); return;
	}
	//*[파일 이름+체크섬 토큰] 원소의 수 파악하기

	FileDirnames_Collect = (FILESTR_DATA*)malloc(sizeof(FILESTR_DATA)*FileDirnames_Collect_Size);
	Set_FileNames(FileDirnames_Collect, FileDirnames_Collect_Size, FileDirnames_Raw);
	free(FileDirnames_Raw);
	//*[파일 이름+체크섬 토큰] 배열 형성 후 세팅하기

	Metadata_Collect_Size = (((Dxa_Head->dir_hd_ofs) - (Dxa_Head->filemt_ofs))) / sizeof(FILE_METADATA);
	if(Metadata_Collect_Size * sizeof(FILE_METADATA) != ((Dxa_Head->dir_hd_ofs) - (Dxa_Head->filemt_ofs))){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 파일 메타데이터가 깨졌습니다."));
		for(unsigned int i = 0;i < FileDirnames_Collect_Size;i++){ free (FileDirnames_Collect[i].big_str); free (FileDirnames_Collect[i].real_str); }
		free(FileDirnames_Collect); free (Files_Buff); free (Dxa_Buff); return;
	}
	//*[메타데이터] 원소 수 파악하기

	Metadata_Collect = (FILE_METADATA*) malloc(sizeof(FILE_METADATA) * Metadata_Collect_Size);
	memcpy (Metadata_Collect, 
		Dxa_Buff+(Dxa_Head->totalmt_ofs + Dxa_Head->filemt_ofs), 
		(sizeof(FILE_METADATA)*Metadata_Collect_Size));
	//*[메타데이터] 배열 세팅하기

	Dir_Idx_Collect_Size = ((Dxa_Head->total_toklen) - (Dxa_Head->dir_hd_ofs)) / sizeof(DIR_IDX);
	if(Dir_Idx_Collect_Size * sizeof(DIR_IDX) != ((Dxa_Head->total_toklen) - (Dxa_Head->dir_hd_ofs))){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 디렉토리 정보가 깨졌습니다."));
		free (Metadata_Collect);
		for(unsigned int i = 0;i < FileDirnames_Collect_Size;i++){ free (FileDirnames_Collect[i].big_str); free (FileDirnames_Collect[i].real_str); }
		free (FileDirnames_Collect); free (Files_Buff); free (Dxa_Buff); return;
	}
	//*[디렉토리 인덱스] 원소 수 파악하기

	Dir_Idx_Collect = (DIR_IDX*) malloc(sizeof(DIR_IDX) * Dir_Idx_Collect_Size);
	memcpy (Dir_Idx_Collect, Dxa_Buff+(Dxa_Head->totalmt_ofs + Dxa_Head->dir_hd_ofs), (sizeof(DIR_IDX)*Dir_Idx_Collect_Size));
	//*[디렉토리 인덱스] 배열 세팅하기

	ParentDir_Name_Len_Buff = (unsigned int*)malloc(sizeof(int) * Dir_Idx_Collect_Size);
	memset (ParentDir_Name_Len_Buff, 0, sizeof(int) * Dir_Idx_Collect_Size);
	//*[부모 디렉토리 이름 길이] 배열 초기화

//	free (Dxa_Buff);
	//*기존 버퍼는 이쯤되면 필요없으니 지운다
	//***1. 헤더의 정보를 바탕으로 각 메타데이터에 대응되는 구조체를 만듦. 사실 복사만 잘 하면 된다




	memset (Root_Dir, 0, MAX_PATH);
	char *tt = U2K (ExtDlg->Wolf_File_Title.GetBuffer());
	for(unsigned int i = 0;i < strlen(tt);i++){ Root_Dir[i] = tt[i]; }
	free(tt);

	Wolf_File_ListName = ExtDlg->Wolf_File_Title + _T(".txt");
	//*미리 루트 디렉토리, 리스트 파일 이름 만들어두기
	
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {
		if (Dir_Idx_Collect[i].parent_dir != -1)  {
			if (Dir_Idx_Collect[i].parent_dir == 0x00) {
				ParentDir_Name_Len_Buff[i] = (unsigned int)strlen(Root_Dir);
				//*부모가 루트 디렉토리라면 그냥 루트 길이만 더해주면 된다
			}
			else{
				Parent_Dir_Idx = (unsigned int)Dir_Idx_Collect[i].parent_dir / sizeof(DIR_IDX);
				//*[디렉토리 인덱스] 내 부모 디렉토리의 위치
				Parent_Dir_FileDirnames_Pos = Find_FileOrDirName_Idx (FileDirnames_Collect, FileDirnames_Collect_Size,
					Metadata_Collect[Dir_Idx_Collect[Parent_Dir_Idx].dirmt_ofs / sizeof(FILE_METADATA)].fltok_start);
				//*부모 디렉토리에 대응되는 [이름+체크섬]이 있는 곳(인덱스)

				ParentDir_Name_Len_Buff[i] = ParentDir_Name_Len_Buff[Parent_Dir_Idx];
				//*부모 디렉토리의 [부모 디렉토리 경로 길이] 더하기
				ParentDir_Name_Len_Buff[i]++;
				//*슬래시('\') 길이 더하기
				ParentDir_Name_Len_Buff[i] += (unsigned int)strlen(FileDirnames_Collect[Parent_Dir_FileDirnames_Pos].real_str);
				//*부모 디렉토리와 대응되는 문자열을 찾아 그 길이 더하기
			}
			//*부모 디렉토리의 길이를 얻어와 슬래시와 함께 더하기
		}//*루트 디렉토리가 아닐 때(루트 디렉토리일 때는 이미 0으로 세팅되어 있으므로 상관없다)
	}
	//*[부모 디렉토리 이름 길이] 배열 세팅하기
	//*맨 처음이 루트 디렉토리임을 알고 해야 한다.(시작 위치 : 0)
	//*다행히도 구조상 처음부터 시행하면 문제없이 길이 파악이 가능하다

	//***1.5. 파일 분해를 기록할 리스트 생성/[부모 디렉토리의 경로 길이] 배열 세팅



	

	unsigned int Total_Cnt = 0, Cnt = 0;
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) { Total_Cnt += Dir_Idx_Collect[i].mt_number; }
	//*프로그레스 바 세팅 준비

	memset(Upper_Dir, 0, MAX_PATH);
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {

		memset(Upper_Dir + ParentDir_Name_Len_Buff[i], MAX_PATH - ParentDir_Name_Len_Buff[i], 0);
		//*상위 디렉토리 세팅 준비
		//*정확히는 부모 디렉토리 경로만 남겨놓아야 한다

		Ofs_Of_DirMeta = 0;
		//*디렉토리 소유 디렉토리로 인한 오차변수 보정 초기화

		if (Dir_Idx_Collect[i].parent_dir == -1) {
			sprintf_s (Upper_Dir, MAX_PATH, "%s", Root_Dir);
			//*상위 디렉토리에 루트 디렉토리 복사
		}//*부모 디렉토리가 없으면 루트 디렉토리임을 의미한다
		else {
			Each_Dir_FileDirnames_Pos = Find_FileOrDirName_Idx (FileDirnames_Collect, FileDirnames_Collect_Size,
				Metadata_Collect[Dir_Idx_Collect[i].dirmt_ofs / sizeof(FILE_METADATA)].fltok_start);
			//*미리 위치를 얻어두기

			sprintf_s (Upper_Dir + ParentDir_Name_Len_Buff[i], 
				MAX_PATH - ParentDir_Name_Len_Buff[i], 
				"\\%s", 
				FileDirnames_Collect[Each_Dir_FileDirnames_Pos].real_str);
			//*출력할 곳에 위치보정을 하고, 슬래시까지 더해서 찍는다
		}//*부모 디렉토리가 있다면 루트 디렉토리의 하위 디렉토리이므로, 부모 디렉토리에 붙여서 출력한다


		memset (List_String, 0, MAX_PATH);
		sprintf_s (List_String, MAX_PATH, "%s\r\n", Upper_Dir);

		if (code == JAP_CODE) { tmpwstr = J2U (List_String); }
		else if (code == KOR_CODE) { tmpwstr = K2U (List_String); }
		Wolf_File_List += tmpwstr; free (tmpwstr);
		//*상위 디렉토리 이름 확정 후 리스트에 기록. 덤으로 개행까지

		if (My_mkdir (Upper_Dir, code) != 0) {
			if (code == JAP_CODE) { tmpwstr = J2U (Upper_Dir); }
			else if (code == KOR_CODE) { tmpwstr = K2U (Upper_Dir); }
			CString t = tmpwstr;
			AfxMessageBox (t + _T(" : 폴더를 만들지 못했습니다."));
			ExtDlg->m_Extract_Progress.SetPos (0);
			Wolf_File_ListName = _T(""); Wolf_File_List.Empty();
			free (tmpwstr); return;
		}
		//*상위 디렉토리 만들기 (언어코드 차로 만들지 못했다면 에러처리)

		FileInDir_Meta_Start_Pos = Dir_Idx_Collect[i].mt_offset / sizeof(FILE_METADATA);
		//*해당 디렉토리가 소유한 파일들 메타데이터 시작지점 얻기

		for (unsigned int j = 0;j < Dir_Idx_Collect[i].mt_number;j++) {

			ExtDlg->m_Extract_Progress.SetPos (PROGRESSVALUE(Cnt++, Total_Cnt));
			//*프로그레스 바 위치 세팅

			Each_FileDirnames_Pos = Find_FileOrDirName_Idx (FileDirnames_Collect, FileDirnames_Collect_Size,
				Metadata_Collect[FileInDir_Meta_Start_Pos + j].fltok_start);
			//*먼저 해당 메타데이터에 대응되는 [파일/디렉토리명+체크섬] 원소 인덱스를 얻는다

			memset(FileOrDir_In_Dir, MAX_PATH, 0);
			sprintf_s (FileOrDir_In_Dir, MAX_PATH, "%s\\%s",
				Upper_Dir, FileDirnames_Collect[Each_FileDirnames_Pos].real_str);
			//*파일/디렉토리명 확정

			memset (List_String, 0, MAX_PATH);
			sprintf_s (List_String, MAX_PATH, "\t%s\r\n", FileOrDir_In_Dir);


			if (code == JAP_CODE) { tmpwstr = J2U (List_String); }
			else if (code == KOR_CODE) { tmpwstr = K2U (List_String); }
			Wolf_File_List += tmpwstr; free (tmpwstr);
			//*리스트에 기록(개행 추가)


			if ((Metadata_Collect [FileInDir_Meta_Start_Pos + j].attr & DIR) != 0) {}
			//*디렉토리일 때는 굳이 크게 신경쓸 필요 없다
			else {
				bool IsDir = false;
				//*울프툴은 그냥 해도 되지만, 던전타운은 기작을 추가해야 한다
				//*중간에 방점 있으면 그냥 파일이라고 치자

				if (!IsDir) {

					if (code == KOR_CODE) { tmpwstr = K2U(FileOrDir_In_Dir); }
					else { tmpwstr = J2U(FileOrDir_In_Dir); }

					CString Sst (_T("Extracting"));
					if (type == DXA_LOW) { Sst += _T(" (Type 1) : "); }
					else if (type == DXA_220) { Sst += _T(" (Type 2) : "); }
					else if (type == DXA_330) { Sst += _T(" (Type 3) : "); }
					else if (type == DXA_THMK) { Sst += _T(" (Type th_l_2) : "); }
					else { Sst += _T(" (Type th_l_64bit) : "); }
					Sst += tmpwstr;

					ExtDlg->SetDlgItemText (IDC_E_TEXT, Sst);
					//*출력 문자열 갱신


					Each_File_Buff = (unsigned char*)malloc (sizeof(char)*Metadata_Collect [FileInDir_Meta_Start_Pos + j].UncompSize);
					Each_File_Size = Metadata_Collect [FileInDir_Meta_Start_Pos + j].UncompSize;

					if (Metadata_Collect [FileInDir_Meta_Start_Pos + j].CompSize == -1) {
						memcpy (Each_File_Buff, 
							Files_Buff + Metadata_Collect [FileInDir_Meta_Start_Pos + j].offset,
							Metadata_Collect [FileInDir_Meta_Start_Pos + j].UncompSize);
						//*기록하게 될 파일 크기 얻기
					}
					else {
						memcpy (Each_File_Buff, 
							Files_Buff + Metadata_Collect [FileInDir_Meta_Start_Pos + j].offset,
							Metadata_Collect [FileInDir_Meta_Start_Pos + j].CompSize);
						//*기록하게 될 파일 크기 얻기
					}
					//*기록하고자 하는 파일 버퍼 복사해오기(압축/비압축 구분)

					Decode (Each_File_Buff, Each_File_Size);
					Each_File_Size = Metadata_Collect [FileInDir_Meta_Start_Pos + j].UncompSize;
					//*압축이 되어있으면 압축 풀기
					
					hEachFile = CreateFileW (tmpwstr, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (!WriteFile (hEachFile, Each_File_Buff, Each_File_Size, &lp_write, NULL)) {
						CString t = tmpwstr;
						AfxMessageBox (t + _T(" : 파일을 만들지 못했습니다."));
						ExtDlg->m_Extract_Progress.SetPos (0);
						Wolf_File_ListName = _T(""); Wolf_File_List.Empty();
						CloseHandle (hEachFile); free (Each_File_Buff);
						free (tmpwstr); return;
					}
					CloseHandle (hEachFile); free (Each_File_Buff);
					//*파일 기록 후 핸들, 버퍼 해제(언어코드 차로 만들지 못했다면 에러처리)
				}

			}
			//*파일일 때
			
			//*디렉토리일 때와 파일일 때를 구분하여 수행한다

		}
		//*디렉토리 인덱스에 기록된 메타데이터 보유 수만큼 뺑뺑 돌면서 파일/디렉토리들을 출력한다

		Wolf_File_List += _T("\r\n");
		//*리스트 개행 정리

		//*맨 위가 루트 디렉토리를 가리키며, 순차적으로 돌면서 출력한다
		//*울프툴은 디렉토리(0x10) 아니면 파일(0x20)로만 구분하는듯?
	}
	//*해당 wolf 파일이 가진 디렉토리 수만큼 반복한다

	ExtDlg->m_Extract_Progress.SetPos (100);
	ExtDlg->SetDlgItemText (IDC_E_TEXT, _T("Extract Finished"));
	//***2. 메타데이터 대응 구조체를 이용해 파일 분해 및 리스트 기록
	//(기존 방식과 차별점 : (ROOT)에는 wolf의 파일명을 그대로 넣고, 뒤이어 내려가는 디렉토리들도 죄다 붙여넣는다)
	//(그러면 기존 폴더에서도 쉽게 패킹을 할 수 있다. 안 들어가도 된다는 뜻.)

	

	
	free (ParentDir_Name_Len_Buff); free (Dir_Idx_Collect); free (Metadata_Collect);
	for(unsigned int i = 0;i < FileDirnames_Collect_Size;i++){ free (FileDirnames_Collect[i].big_str); free (FileDirnames_Collect[i].real_str); }
	free (FileDirnames_Collect); //free (Files_Buff);
	free (Dxa_Buff);
	//***3. 전부 해제
}




void extracting_64bit (Extract_Dialog* ExtDlg, unsigned int dxa_type, unsigned int code)
{
	HANDLE hDxa;
	unsigned char *Dxa_Buff, *Dxa_Buff_Pnt;
	unsigned int Dxa_Size;						//64비트식이지만 일단은 int형으로 쓴다
	WOLF_BIG_HEADER *Dxa_Head;					//wolf 파일의 헤더

	unsigned int Xor_Ofs;						//메타데이터를 정상화시키기 위해 xor한 횟수를 저장한다

	unsigned char *Files_Buff;					//파일들을 한데 모아놓은 버퍼
	unsigned int FilesBuff_Size;				//파일 버퍼의 크기

	unsigned char *FileDirnames_Raw;			//파일이름 토큰을 곧바로 변화시킬 수 없으므로. 정보를 읽어들인 다음 파일이름 정보로 변환한다.
	FILESTR_DATA *FileDirnames_Collect;			//위에서 파일이름 토큰을 변화시켜 저장할 곳(루트 디렉토리부터 시작, 루트는 없는 이름 취급)
	unsigned int FileDirnames_Collect_Size;		//파일이름 토큰들을 저장할 때 그 갯수

	FILE_BIG_METADATA *Metadata_Collect;		//파일의 메타데이터를 저장할 곳
	unsigned int Metadata_Collect_Size;			//파일이름 토큰들을 저장할 때 그 갯수

	DIR_BIG_IDX *Dir_Idx_Collect;				//디렉토리의 정보를 저장할 곳
	unsigned int *ParentDir_Name_Len_Buff;		//각 부모 디렉토리의 길이를 담아놓는 곳
	unsigned int Dir_Idx_Collect_Size;			//디렉토리 정보를 저장할 때 그 갯수

	unsigned int Parent_Dir_Idx;				//[디렉토리 인덱스] 내의 부모 디렉토리의 위치를 가리킨다
	unsigned int Parent_Dir_FileDirnames_Pos;	//부모 디렉토리의 [이름+체크섬] 위치를 가리킨다
	unsigned int Each_Dir_FileDirnames_Pos;		//자신 디렉토리의 [이름+체크섬] 위치를 가리킨다
	unsigned int Each_FileDirnames_Pos;			//자신(파일 or 디렉토리)의 [이름+체크섬] 위치를 가리킨다
	unsigned int FileInDir_Meta_Start_Pos;		//디렉토리가 소유하는 파일들 메타데이터의 시작지점
	unsigned int Ofs_Of_DirMeta;				//디렉토리가 소유한 디렉토리 메타데이터로 인한 오차 보정 변수

	//*일단 __int64로 해야 맞는 것 같긴 한데, 아무래도 어긋날 우려가 있어 unsigned int로 통일한다

	char Root_Dir [MAX_PATH];					//루트 디렉토리의 경로를 저장할 변수
	char Upper_Dir [MAX_PATH];					//상위 디렉토리의 경로를 저장할 변수
	char FileOrDir_In_Dir [MAX_PATH];			//디렉토리 밑에 속한 파일/디렉토리의 경로를 저장할 변수

	char List_String [MAX_PATH];				//리스트에 로그를 저장하기 위해 쓰이는 버퍼

	unsigned char *Each_File_Buff;				//각 파일을 가리키게 될 버퍼
	unsigned int Each_File_Size;				//각 파일의 크기

	HANDLE hEachFile;							//각 파일을 기록할 때 쓰이는 핸들
		
	DWORD lp_read, lp_write;

	wchar_t *tmpwstr;							//*있으면 써먹기 좋음

	unsigned char *XORKEY;
	if (dxa_type == DXA_330) { XORKEY = WOLF_KEY3; }
	else if (dxa_type == DXA_THMK_64) { XORKEY = THMK2_KEY; }
	else { XORKEY = WOLF_KEY3; }



	hDxa = CreateFileW(((wchar_t*)(LPCTSTR)(ExtDlg->Wolf_Full_Path)), 
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hDxa == INVALID_HANDLE_VALUE){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 dxa 파일을 열지 못했습니다."));
		return;
	}
	//*dxa 파일 열기

	Dxa_Size = GetFileSize(hDxa, NULL);
	if(Dxa_Size <= sizeof(WOLF_BIG_HEADER)){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 dxa 파일이 아닙니다."));
		CloseHandle(hDxa);return;
	}//*dxa 파일 크기 조사. 최소한 헤더보다는 커야 한다

	Dxa_Buff = (unsigned char*)malloc(sizeof(char) * Dxa_Size);
	Dxa_Buff_Pnt = Dxa_Buff;
	if (ReadFile (hDxa, Dxa_Buff, (DWORD)Dxa_Size, &lp_read, NULL) == 0) {
#ifdef WIN64
		AfxMessageBox (_T(" : 용량이 지나치게 커서 분해할 수 없습니다."));
#else
		AfxMessageBox (_T(" : 해당 파일은 용량이 너무 큽니다. 64비트로 돌리십시오."));
#endif
		return;
	}
	CloseHandle (hDxa);
	for (unsigned int i = 0;i < sizeof(WOLF_BIG_HEADER);i++) { Dxa_Buff[i] ^= XORKEY[i%KEY_SIZE]; }
	//*전부 읽어들인 후 핸들 닫기, 그리고 헤더만 xor 까기
	//*그런데 여기서 할당할 때 32비트 프로그램이라 64비트인 __int64와 범위차이가 나는데,
	//*일단 이건 넘기다시피 하고 생각해야겠음.

	Dxa_Head = (WOLF_BIG_HEADER*)Dxa_Buff;
	if ((Dxa_Head->magic != WOLF_MAGIC_64) && (Dxa_Head->magic != DXA_MAGIC_64)) {
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 데이터 파일이 아닙니다."));
		free (Dxa_Buff); return;
	}
	//*헤더 포인터를 만들어 맨 앞쪽을 가리키게 한 후 매직값 조사

	if(Dxa_Size < ((unsigned int)(Dxa_Head->totalmt_ofs) + Dxa_Head->total_toklen)){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 dxa 파일이 아닙니다."));
		free (Dxa_Buff); return;
	}
	//*헤더의 크기 조건 조사

	if (((unsigned int)Dxa_Head->end != HEAD_END) && ((unsigned int)Dxa_Head->end != HEAD_END2)) {
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 dxa 파일의 헤더가 깨졌습니다."));
		free (Dxa_Buff); return;
	}
	//*헤더 끝부분 조사

	if(((__int64)(Dxa_Head->total_toklen) <= Dxa_Head->filemt_ofs) || 
		((__int64)(Dxa_Head->total_toklen) <= Dxa_Head->dir_hd_ofs)){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 dxa 파일이 아닙니다."));
		free (Dxa_Buff); return;
	}
	//*각 메타데이터들간의 오프셋 관계 조사

	Xor_Ofs = (unsigned int)(Dxa_Head->totalmt_ofs)%KEY_SIZE;
	for (__int64 i = Dxa_Head->totalmt_ofs;i < Dxa_Size;i++) {
		Dxa_Buff[i] ^= XORKEY[((i+(KEY_SIZE-Xor_Ofs))%KEY_SIZE)];
	}
	//*xor을 할 때, 오프셋의 기준이 되는 totalmt_ofs에 맞추어 오프셋을 계산해야 한다
	if (*(unsigned int*)(Dxa_Buff + (unsigned int)(Dxa_Head->totalmt_ofs)) != 0x00000000) {
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 해당 파일은 정상적인 dxa 파일이 아닙니다."));
		free (Dxa_Buff); return;
		//*이래도 0이 아니라면 에러처리한다
	}
	//*메타데이터 부분 xor 까두기(리스트 기록용)
	//***0. 분해 시작 전의 선제 조건 검사(메타데이터 시작은 무조건 루트부터 시작하기에 0이어야 한다)

	FilesBuff_Size = (unsigned int)((Dxa_Head->totalmt_ofs) - (Dxa_Head->header_size));

	Files_Buff = Dxa_Buff+(unsigned int)(Dxa_Head->header_size);
	//*[파일 버퍼] 크기 파악 및 복사(xor은 파일 기록 시 함께 한다)
	//*(너무 크게 할당하면 실패하기도 하나 보다. 그러니 포인터로서 이용하게 한다)
	
	FileDirnames_Raw = (unsigned char*)malloc(sizeof(char)*(unsigned int)(Dxa_Head->filemt_ofs));
	memcpy (FileDirnames_Raw, 
		Dxa_Buff+(unsigned int)(Dxa_Head->totalmt_ofs), 
		(unsigned int)(Dxa_Head->filemt_ofs));
	FileDirnames_Collect_Size = Get_File_Count(FileDirnames_Raw, (unsigned int)(Dxa_Head->filemt_ofs));
	if(FileDirnames_Collect_Size == 0xFFFFFFFF){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 파일 이름 토큰이 깨졌습니다."));
		free (FileDirnames_Raw); free (Files_Buff); free (Dxa_Buff); return;
	}
	//*[파일 이름+체크섬 토큰] 원소의 수 파악하기

	FileDirnames_Collect = (FILESTR_DATA*)malloc(sizeof(FILESTR_DATA)*FileDirnames_Collect_Size);
	Set_FileNames(FileDirnames_Collect, FileDirnames_Collect_Size, FileDirnames_Raw);
	free(FileDirnames_Raw);
	//*[파일 이름+체크섬 토큰] 배열 형성 후 세팅하기

	Metadata_Collect_Size = ((unsigned int)((Dxa_Head->dir_hd_ofs) - (Dxa_Head->filemt_ofs))) / sizeof(FILE_BIG_METADATA);
	if(Metadata_Collect_Size * sizeof(FILE_BIG_METADATA) != (unsigned int)((Dxa_Head->dir_hd_ofs) - (Dxa_Head->filemt_ofs))){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 파일 메타데이터가 깨졌습니다."));
		for(unsigned int i = 0;i < FileDirnames_Collect_Size;i++){ free (FileDirnames_Collect[i].big_str); free (FileDirnames_Collect[i].real_str); }
		free(FileDirnames_Collect); free (Files_Buff); free (Dxa_Buff); return;
	}
	//*[메타데이터] 원소 수 파악하기

	Metadata_Collect = (FILE_BIG_METADATA*) malloc(sizeof(FILE_BIG_METADATA) * Metadata_Collect_Size);
	memcpy (Metadata_Collect, 
		Dxa_Buff+(unsigned int)(Dxa_Head->totalmt_ofs + Dxa_Head->filemt_ofs), 
		(sizeof(FILE_BIG_METADATA)*Metadata_Collect_Size));
	//*[메타데이터] 배열 세팅하기

	Dir_Idx_Collect_Size = (unsigned int)((Dxa_Head->total_toklen) - (Dxa_Head->dir_hd_ofs)) / sizeof(DIR_BIG_IDX);
	if(Dir_Idx_Collect_Size * sizeof(DIR_BIG_IDX) != (unsigned int)((Dxa_Head->total_toklen) - (Dxa_Head->dir_hd_ofs))){
		AfxMessageBox (ExtDlg->Wolf_File_Name + _T(" : 디렉토리 정보가 깨졌습니다."));
		free (Metadata_Collect);
		for(unsigned int i = 0;i < FileDirnames_Collect_Size;i++){ free (FileDirnames_Collect[i].big_str); free (FileDirnames_Collect[i].real_str); }
		free (FileDirnames_Collect); free (Files_Buff); free (Dxa_Buff); return;
	}
	//*[디렉토리 인덱스] 원소 수 파악하기

	Dir_Idx_Collect = (DIR_BIG_IDX*) malloc(sizeof(DIR_BIG_IDX) * Dir_Idx_Collect_Size);
	memcpy (Dir_Idx_Collect,
		Dxa_Buff+(unsigned int)(Dxa_Head->totalmt_ofs + Dxa_Head->dir_hd_ofs), 
		(sizeof(DIR_BIG_IDX)*Dir_Idx_Collect_Size));
	//*[디렉토리 인덱스] 배열 세팅하기

	ParentDir_Name_Len_Buff = (unsigned int*)malloc(sizeof(int) * Dir_Idx_Collect_Size);
	memset (ParentDir_Name_Len_Buff, 0, sizeof(int) * Dir_Idx_Collect_Size);
	//*[부모 디렉토리 이름 길이] 배열 초기화

//	free (Dxa_Buff);
	//*기존 버퍼는 이쯤되면 필요없으니 지운다 => 마지막에 지운다. 파일이 너무 커지면 한 번만 읽어야 함
	//***1. 헤더의 정보를 바탕으로 각 메타데이터에 대응되는 구조체를 만듦. 사실 복사만 잘 하면 된다




	
	memset (Root_Dir, 0, MAX_PATH);
	char *tt = U2K (ExtDlg->Wolf_File_Title.GetBuffer());
	for(unsigned int i = 0;i < strlen(tt);i++){ Root_Dir[i] = tt[i]; }
	free(tt);

	Wolf_File_ListName = ExtDlg->Wolf_File_Title + _T(".txt");
	//*미리 루트 디렉토리, 리스트 파일 이름 만들어두기
	
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {
		if (Dir_Idx_Collect[i].parent_dir != (__int64)(-1))  {
			if (Dir_Idx_Collect[i].parent_dir == 0x00) {
				ParentDir_Name_Len_Buff[i] = (unsigned int)strlen(Root_Dir);
				//*부모가 루트 디렉토리라면 그냥 루트 길이만 더해주면 된다
			}
			else{
				Parent_Dir_Idx = (unsigned int)Dir_Idx_Collect[i].parent_dir / sizeof(DIR_BIG_IDX);
				//*[디렉토리 인덱스] 내 부모 디렉토리의 위치
				Parent_Dir_FileDirnames_Pos = Find_FileOrDirName_Idx (FileDirnames_Collect, FileDirnames_Collect_Size,
					(unsigned int)Metadata_Collect[(unsigned int)Dir_Idx_Collect[Parent_Dir_Idx].dirmt_ofs / sizeof(FILE_BIG_METADATA)].fltok_start);
				//*부모 디렉토리에 대응되는 [이름+체크섬]이 있는 곳(인덱스)

				ParentDir_Name_Len_Buff[i] = ParentDir_Name_Len_Buff[Parent_Dir_Idx];
				//*부모 디렉토리의 [부모 디렉토리 경로 길이] 더하기
				ParentDir_Name_Len_Buff[i]++;
				//*슬래시('\') 길이 더하기
				ParentDir_Name_Len_Buff[i] += (unsigned int)strlen(FileDirnames_Collect[Parent_Dir_FileDirnames_Pos].real_str);
				//*부모 디렉토리와 대응되는 문자열을 찾아 그 길이 더하기
			}
			//*부모 디렉토리의 길이를 얻어와 슬래시와 함께 더하기
		}//*루트 디렉토리가 아닐 때(루트 디렉토리일 때는 이미 0으로 세팅되어 있으므로 상관없다)
	}
	//*[부모 디렉토리 이름 길이] 배열 세팅하기
	//*맨 처음이 루트 디렉토리임을 알고 해야 한다.(시작 위치 : 0)
	//*다행히도 구조상 처음부터 시행하면 문제없이 길이 파악이 가능하다

	//***1.5. 파일 분해를 기록할 리스트 생성/[부모 디렉토리의 경로 길이] 배열 세팅




	__int64 Total_Cnt = 0, Cnt = 0;
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) { Total_Cnt += Dir_Idx_Collect[i].mt_number; }
	//*프로그레스 바 세팅 준비

	memset (Upper_Dir, 0, MAX_PATH);
	for (unsigned int i = 0;i < Dir_Idx_Collect_Size;i++) {

		memset(Upper_Dir + ParentDir_Name_Len_Buff[i], MAX_PATH - ParentDir_Name_Len_Buff[i], 0);
		//*상위 디렉토리 세팅 준비
		//*정확히는 부모 디렉토리 경로만 남겨놓아야 한다

		Ofs_Of_DirMeta = 0;
		//*디렉토리 소유 디렉토리로 인한 오차변수 보정 초기화

		if (Dir_Idx_Collect[i].parent_dir == (__int64)-1) {
			sprintf_s (Upper_Dir, MAX_PATH, "%s", Root_Dir);
			//*상위 디렉토리에 루트 디렉토리 복사
		}//*부모 디렉토리가 없으면 루트 디렉토리임을 의미한다
		else {
			Each_Dir_FileDirnames_Pos = Find_FileOrDirName_Idx (FileDirnames_Collect, FileDirnames_Collect_Size,
				(unsigned int)Metadata_Collect[(unsigned int)Dir_Idx_Collect[i].dirmt_ofs / sizeof(FILE_BIG_METADATA)].fltok_start);
			//*미리 위치를 얻어두기

			sprintf_s (Upper_Dir + ParentDir_Name_Len_Buff[i], 
				MAX_PATH - ParentDir_Name_Len_Buff[i], 
				"\\%s", 
				FileDirnames_Collect[Each_Dir_FileDirnames_Pos].real_str);
			//*출력할 곳에 위치보정을 하고, 슬래시까지 더해서 찍는다
		}//*부모 디렉토리가 있다면 루트 디렉토리의 하위 디렉토리이므로, 부모 디렉토리에 붙여서 출력한다


		memset (List_String, 0, MAX_PATH);
		sprintf_s (List_String, MAX_PATH, "%s\r\n", Upper_Dir);

		if (code == JAP_CODE) { tmpwstr = J2U (List_String); }
		else if (code == KOR_CODE) { tmpwstr = K2U (List_String); }
		Wolf_File_List += tmpwstr; free (tmpwstr);
		//*상위 디렉토리 이름 확정 후 리스트에 기록. 덤으로 개행까지


		if (My_mkdir (Upper_Dir, code) != 0) {
			if (code == JAP_CODE) { tmpwstr = J2U (Upper_Dir); }
			else if (code == KOR_CODE) { tmpwstr = K2U (Upper_Dir); }
			CString t = tmpwstr;
			AfxMessageBox (t + _T(" : 폴더를 만들지 못했습니다."));
			ExtDlg->m_Extract_Progress.SetPos (0);
			Wolf_File_ListName = _T(""); Wolf_File_List.Empty();
			free (tmpwstr); return;
		}
		//*상위 디렉토리 만들기(만들지 못했다면 에러처리)


		FileInDir_Meta_Start_Pos = (unsigned int)Dir_Idx_Collect[i].mt_offset / sizeof(FILE_BIG_METADATA);
		//*해당 디렉토리가 소유한 파일들 메타데이터 시작지점 얻기

		for (unsigned int j = 0;j < (unsigned int)Dir_Idx_Collect[i].mt_number;j++) {

			ExtDlg->m_Extract_Progress.SetPos (PROGRESSVALUE(Cnt++, Total_Cnt));
			//*프로그레스 바 위치 세팅

			Each_FileDirnames_Pos = Find_FileOrDirName_Idx (FileDirnames_Collect, FileDirnames_Collect_Size,
				(unsigned int)Metadata_Collect[FileInDir_Meta_Start_Pos + j].fltok_start);
			//*먼저 해당 메타데이터에 대응되는 [파일/디렉토리명+체크섬] 원소 인덱스를 얻는다

			memset(FileOrDir_In_Dir, MAX_PATH, 0);
			sprintf_s (FileOrDir_In_Dir, MAX_PATH, "%s\\%s",
				Upper_Dir, FileDirnames_Collect[Each_FileDirnames_Pos].real_str);
			//*파일/디렉토리명 확정

			memset (List_String, 0, MAX_PATH);
			sprintf_s (List_String, MAX_PATH, "\t%s\r\n", FileOrDir_In_Dir);
			if (code == JAP_CODE) { tmpwstr = J2U (List_String); }
			else if (code == KOR_CODE) { tmpwstr = K2U (List_String); }
			Wolf_File_List += tmpwstr; free (tmpwstr);
			//*리스트에 기록(개행 추가)

			if ((Metadata_Collect [FileInDir_Meta_Start_Pos + j].attr & (__int64)(DIR)) != 0) {

				//*디렉토리를 이 때 만들지 않고, 소유 파일수만큼 더해 넘길..필요 없을지도 모른다

			}
			//*디렉토리일 때

			else {

				bool IsDir = false;
				//*울프툴은 그냥 해도 되지만, 던전타운은 기작을 추가해야 한다
				//*중간에 방점 있으면 그냥 파일이라고 치자

				if (!IsDir) {


					if (code == KOR_CODE) { tmpwstr = K2U(FileOrDir_In_Dir); }
					else { tmpwstr = J2U(FileOrDir_In_Dir); }

					CString Sst (_T("Extracting (Type 3) : "));
					Sst += tmpwstr;

					ExtDlg->SetDlgItemText (IDC_E_TEXT, Sst);
					//*출력 문자열 갱신


					Each_File_Size = (unsigned int)Metadata_Collect [FileInDir_Meta_Start_Pos + j].UncompSize;
					//*기록하게 될 파일 크기 얻기

					Xor_Ofs = Each_File_Size%KEY_SIZE;
					//*xor 오프셋 보정값

					if (Metadata_Collect [FileInDir_Meta_Start_Pos + j].CompSize != (__int64)-1) {
						Each_File_Buff = (unsigned char*)malloc (sizeof(char) * Each_File_Size);
						memset (Each_File_Buff, 0, Each_File_Size);

						for (unsigned int jj = 0;jj < (unsigned int)Metadata_Collect [FileInDir_Meta_Start_Pos + j].CompSize;jj++) { 
							(Files_Buff + (unsigned int)Metadata_Collect [FileInDir_Meta_Start_Pos + j].offset)[jj] ^= XORKEY[(jj+Xor_Ofs)%KEY_SIZE]; 
						}
						//*오프셋을 보정한 후 xor 까기

						memcpy (Each_File_Buff, 
							Files_Buff + (unsigned int)Metadata_Collect [FileInDir_Meta_Start_Pos + j].offset,
							(unsigned int)Metadata_Collect [FileInDir_Meta_Start_Pos + j].CompSize);
						//*압축 풀 버퍼로 복사

						Decode (Each_File_Buff, Each_File_Size);
						//*해당 위치로 들어가 새로 압축을 풀어준다
					}
					//*압축시
					else {
						Each_File_Buff = Files_Buff + (unsigned int)Metadata_Collect [FileInDir_Meta_Start_Pos + j].offset;
						//*해당 위치로 들어가 그대로 개별 파일 버프 포인터를 위치시킨다

						for (unsigned int jj = 0;jj < Each_File_Size;jj++) { Each_File_Buff[jj] ^= XORKEY[(jj+Xor_Ofs)%KEY_SIZE]; }
						//*오프셋을 보정한 후 xor 까기
					}
					//*비압축시
					//*압축과 비압축을 구별할 수 있다(압축 시에는 압축크기/비압축크기가 다 적혀 있다)
					
					hEachFile = CreateFileW (tmpwstr, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (!WriteFile (hEachFile, Each_File_Buff, Each_File_Size, &lp_write, NULL)) {
						CString t = tmpwstr;
						AfxMessageBox (t + _T(" : 파일을 만들지 못했습니다."));
						ExtDlg->m_Extract_Progress.SetPos (0);
						Wolf_File_ListName = _T(""); Wolf_File_List.Empty();
						if (Metadata_Collect [FileInDir_Meta_Start_Pos + j].CompSize != (__int64)-1) { free (Each_File_Buff); }
						CloseHandle (hEachFile); free (tmpwstr); return;
					}
					CloseHandle (hEachFile);
					if (Metadata_Collect [FileInDir_Meta_Start_Pos + j].CompSize != (__int64)-1) { free (Each_File_Buff); }
					//*파일 기록 후 핸들, 버퍼 해제(만들지 못했다면 에러처리)
				}

			}
			//*파일일 때
			
			//*디렉토리일 때와 파일일 때를 구분하여 수행한다

		}
		//*디렉토리 인덱스에 기록된 메타데이터 보유 수만큼 뺑뺑 돌면서 파일/디렉토리들을 출력한다

		Wolf_File_List += _T("\r\n");
		//*리스트 개행 정리

		//*맨 위가 루트 디렉토리를 가리키며, 순차적으로 돌면서 출력한다
	}
	//*해당 wolf 파일이 가진 디렉토리 수만큼 반복한다

	ExtDlg->m_Extract_Progress.SetPos (100);
	ExtDlg->SetDlgItemText (IDC_E_TEXT, _T("Extract Finished"));
	//***2. 메타데이터 대응 구조체를 이용해 파일 분해 및 리스트 기록
	//(기존 방식과 차별점 : (ROOT)에는 wolf의 파일명을 그대로 넣고, 뒤이어 내려가는 디렉토리들도 죄다 붙여넣는다)
	//(그러면 기존 폴더에서도 쉽게 패킹을 할 수 있다. 안 들어가도 된다는 뜻.)

	

	
	free (ParentDir_Name_Len_Buff); free (Dir_Idx_Collect); free (Metadata_Collect);
	for(unsigned int i = 0;i < FileDirnames_Collect_Size;i++){ free (FileDirnames_Collect[i].big_str); free (FileDirnames_Collect[i].real_str); }
	free (FileDirnames_Collect); //free (Files_Buff);
	free (Dxa_Buff);
	//***3. 전부 해제
}