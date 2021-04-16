#include "stdafx.h"

#include "Wolf_Functions.h"


int My_mkdir (char *Dir_buffer, unsigned int code) {
	wchar_t *ts;

	if (code == KOR_CODE) { ts = K2U(Dir_buffer); }
	else { ts = J2U(Dir_buffer); }

	int ret = _wmkdir(ts);	//디렉토리 만들기(문자열을 유니코드로 바꿔야 할듯)
	free(ts);
	return ret;
	//*s-jis에서 유니코드로 변환하여 폴더를 만들도록 한다.
	//*이는 나중에 바뀔 수도 있다
}

int FD_Rename (wchar_t *OldFileName, wchar_t *NewFileName)
{
	unsigned int ret;
	unsigned int OldSlashNum, NewSlashNum;
	unsigned int OldSlashPoint, NewSlashPoint;
	unsigned int OldLen, NewLen, OldAccu, NewAccu;
	TCHAR tmpBuff [MAX_PATH];
	memcpy (tmpBuff, OldFileName, sizeof(TCHAR)*MAX_PATH);

	OldLen = (unsigned int)wcslen (OldFileName);
	NewLen = (unsigned int)wcslen (NewFileName);

	OldSlashNum = NewSlashNum = 0;
	for (unsigned int i = 0;(i < OldLen) || (i < NewLen);i++) {
		if (i < OldLen) { if ((OldFileName[i] == '\\') || (OldFileName[i] == '/')) { OldSlashNum++; } }
		if (i < NewLen) { if ((NewFileName[i] == '\\') || (NewFileName[i] == '/')) { NewSlashNum++; } }
	}
	if (OldSlashNum != NewSlashNum) { return -1; }
	//각 슬래시의 수를 파악하고, 같지 않으면 -1을 반환

	OldSlashPoint = NewSlashPoint = 0;
	OldAccu = NewAccu = 0;
	if (OldSlashNum > 0) {
		for (unsigned int i = 0;i < OldSlashNum;i++) {

			for (unsigned int j = OldAccu;j < OldLen;j++) {
				if ((tmpBuff[j] == '\\') || (tmpBuff[j] == '/')) { 
					OldSlashPoint = j; OldAccu = j+1;
					break;
				} 
			}
			for (unsigned int j = NewAccu;j < NewLen;j++) {
				if ((NewFileName[j] == '\\') || (NewFileName[j] == '/')) { 
					NewSlashPoint = j; NewAccu = j+1;
					break;
				} 
			}
			//슬래시 지점 지정

			tmpBuff[OldSlashPoint] = NewFileName[NewSlashPoint] = 0;
			//슬래시 없애기

			if (_tcscmp (tmpBuff, NewFileName) != 0) {
				_wrename(tmpBuff, NewFileName);
				memcpy (tmpBuff, NewFileName, sizeof(TCHAR)*_tcslen(NewFileName));
			}
			//rename으로 디렉토리 먼저 바꾸기

			tmpBuff[OldSlashPoint] = NewFileName[NewSlashPoint] = '/';
			//다시 회복시킴

		}
		//슬래시별로 딱딱 끊어가며, 디렉토리부터 순서대로 바꾼다
	}

	ret = _wrename (tmpBuff, NewFileName);


	return ret;
	//디렉토리까지 갱신시키는 rename
	//만일 '\'의 수가 서로 차이가 날 경우 실패하도록 한다
}




unsigned int Get_File_Count(unsigned char *filenam_tok, unsigned int filenam_tok_size){
	unsigned int cnt = 0;
	unsigned char *tok_offset = filenam_tok;
	unsigned short fl_len, chksum, chksum_tp;
	while(tok_offset < (filenam_tok+filenam_tok_size)){
		cnt++;
		fl_len = *(unsigned short*)tok_offset;tok_offset += sizeof(short);
		chksum_tp = *(unsigned short*)tok_offset;tok_offset += sizeof(short);
		chksum = 0;
		for(unsigned short i = 0;i < fl_len*4;i++){chksum += *(tok_offset+i);}
		if(chksum_tp != chksum){return 0xFFFFFFFF;}
		tok_offset += (fl_len*8);
		if(tok_offset > (filenam_tok+filenam_tok_size)){return 0xFFFFFFFF;}
	}
	return cnt;
}

void Set_FileNames(FILESTR_DATA *filenam_coll, unsigned int filenam_coll_size, unsigned char *filenam_tok){
	unsigned int i;
	unsigned char *tok_offset = filenam_tok;
	for(i = 0;i < filenam_coll_size;i++){
		filenam_coll[i].start = (unsigned int)(tok_offset-filenam_tok);
		filenam_coll[i].len = *(unsigned short*)tok_offset;tok_offset += sizeof(short);

		filenam_coll[i].chksum = *(unsigned short*)tok_offset;tok_offset += sizeof(short);

		if(filenam_coll[i].len == 0){
			filenam_coll[i].big_str = (char*)malloc(sizeof(char));
			filenam_coll[i].big_str[0] = 0;
		}
		else{
			filenam_coll[i].big_str = (char*)malloc(sizeof(char)*filenam_coll[i].len*4);
			for(unsigned short j = 0;j < (filenam_coll[i].len*4);j++){filenam_coll[i].big_str[j] = (char)(tok_offset[j]);}
			tok_offset += (sizeof(char)*filenam_coll[i].len*4);
		}

		if(filenam_coll[i].len == 0){
			filenam_coll[i].real_str = (char*)malloc(sizeof(char));
			filenam_coll[i].real_str[0] = 0;
		}
		else{
			filenam_coll[i].real_str = (char*)malloc(sizeof(char)*filenam_coll[i].len*4);
			for(unsigned short j = 0;j < (filenam_coll[i].len*4);j++){filenam_coll[i].real_str[j] = (char)(tok_offset[j]);}
			tok_offset += (sizeof(char)*filenam_coll[i].len*4);
		}
	}
}

int Get_Last_SlashPos (char *pathname, unsigned int code){
	
	if(strlen(pathname) == 0){return -1;}

	if (code == KOR_CODE) {
		wchar_t *t = K2U (pathname);
		for(int i = (int)wcslen(t)-1;i >= 0;i--){
			if(t[i] == '\\'){ t[i] = ':'; break; }
		}

		char *ut = U2K (t);
		for(int i = (int)strlen(ut)-1;i >= 0;i--){
			if(ut[i] == ':'){ free (t); free (ut); return i; }
		}
		free (ut);
	}
	else {
		wchar_t *t = J2U (pathname);
		for(int i = (int)wcslen(t)-1;i >= 0;i--){
			if(t[i] == '\\'){ t[i] = ':'; break; }
		}

		char *ut = U2J (t);
		for(int i = (int)strlen(ut)-1;i >= 0;i--){
			if(ut[i] == ':'){ free (t); free (ut); return i; }
		}
		free (ut);
	}

	return 0;
}
//이게 상당히 까다롭다. 그러므로 유니코드 변환 후 '?'로 바꾸고 다시 멀티코드로 변환하는 게 좋은 방법일 듯 싶다

int Is_File_Or_Dir (wchar_t* ss) {
	_wfinddatai64_t c_file;
	intptr_t hFile;
	int result;

	if((hFile = _wfindfirsti64(ss, &c_file)) == -1L){result = -1;} // 파일 또는 디렉토리가 없으면 -1 반환
	else{
		if (c_file.attrib & _A_SUBDIR){result = 0;}// 디렉토리면 0 반환
		else{result = 1;}// 그밖의 경우는 "존재하는 파일"이기에 1 반환
	}
	_findclose(hFile);
	return result;
}

unsigned int Find_FileOrDirName_Idx (FILESTR_DATA *fsd, unsigned int fsd_size, unsigned int pos)
{
	unsigned int i, accm_val;

	accm_val = 0;
	for(i = 0;i < fsd_size;i++){
		if(pos == accm_val){ return i; }
		accm_val += (sizeof(short)*2);
		accm_val += (fsd[i].len*4*2);
	}
	return i;
}

void Converting_Big (char *dest, char *source, unsigned int length, unsigned int code)
{
	if (code == KOR_CODE) {
		wchar_t *t = K2U(source);
		for(unsigned int i = 0;i < length;i++){
			if((t[i] >= 'a') && (t[i] <= 'z')){ t[i] = t[i]-('a'-'A'); }
		}

		char *s = U2K (t); free (t);
		for(unsigned int i = 0;i < length;i++){ dest[i] = s[i]; } free (s);
	}
	else {
		wchar_t *t = J2U(source);
		for(unsigned int i = 0;i < length;i++){
			if((t[i] >= 'a') && (t[i] <= 'z')){ t[i] = t[i]-('a'-'A'); }
		}

		char *s = U2J (t); free (t);
		for(unsigned int i = 0;i < length;i++){ dest[i] = s[i]; } free (s);
	}
}




void Low_GameDat_Xor (unsigned char *Buff, unsigned int BuffSize)
{
	unsigned int Start_Idx = 0xA;
	unsigned char FirstChar = Buff[0];
	unsigned char SecondChar = Buff[8];
	unsigned char ThirdChar = Buff[6];
	unsigned int tempint;
	
	tempint = FirstChar;
	for (unsigned int i = Start_Idx;i < BuffSize;i++) {
		tempint = (tempint*0x343FD) + 0x269EC3;
		Buff[i] ^= (((tempint>>0x10) & 0x7000) >> 0xC);
	}
	
	tempint = SecondChar;
	for (unsigned int i = Start_Idx;i < BuffSize;i += 2) {
		tempint = (tempint*0x343FD) + 0x269EC3;
		Buff[i] ^= (((tempint>>0x10) & 0x7000) >> 0xC);
	}
	
	tempint = ThirdChar;
	for (unsigned int i = Start_Idx;i < BuffSize;i += 5) {
		tempint = (tempint*0x343FD) + 0x269EC3;
		Buff[i] ^= (((tempint>>0x10) & 0x7000) >> 0xC);
	}
}
//게임 버전이 낮을 때 글씨체와 게임제목을 바꿀 수 있게 한다







wchar_t* J2U (char *string)
{
	wchar_t *bstr;
	int nLen = MultiByteToWideChar(932, 0, string, -1, NULL, NULL);
	bstr = (wchar_t*)malloc(sizeof(wchar_t)*(nLen+1));
	bstr[nLen] = 0;
	MultiByteToWideChar(932, 0, string, -1, bstr, nLen);
	return bstr;
}

wchar_t* K2U (char *string)
{
	wchar_t *bstr;
	int nLen = MultiByteToWideChar(949, 0, string, -1, NULL, NULL);
	bstr = (wchar_t*)malloc(sizeof(wchar_t)*(nLen+1));
	bstr[nLen] = 0;
	MultiByteToWideChar(949, 0, string, -1, bstr, nLen);
	return bstr;
}

char* U2K (wchar_t *string)
{
	char *lstr;
	int nLen = WideCharToMultiByte(949, 0, string, -1, NULL, NULL, NULL, NULL);
	lstr = (char*)malloc(sizeof(char)*(nLen+1));
	lstr[nLen] = 0;
	WideCharToMultiByte(949, 0, string, -1, lstr, nLen, NULL, NULL);
	return lstr;
}

char* U2J (wchar_t *string)
{
	char *lstr;
	int nLen = WideCharToMultiByte(932, 0, string, -1, NULL, NULL, NULL, NULL);
	lstr = (char*)malloc(sizeof(char)*(nLen+1));
	lstr[nLen] = 0;
	WideCharToMultiByte(932, 0, string, -1, lstr, nLen, NULL, NULL);
	return lstr;
}

wchar_t* UTF8_To_UTF16 (char *str)
{
	wchar_t *bstr;
	int nLen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, NULL);
	bstr = (wchar_t*)malloc(sizeof(wchar_t)*(nLen+1));
	bstr[nLen] = 0;
	MultiByteToWideChar(CP_UTF8, 0, str, -1, bstr, nLen);
	return bstr;
}

char* UTF16_To_UTF8 (wchar_t *str)
{
	char *lstr;
	int nLen = WideCharToMultiByte (CP_UTF8, 0, str, -1, NULL, NULL, NULL, NULL);
	lstr = (char*)malloc(sizeof(char)*(nLen+1));
	lstr[nLen] = 0;
	WideCharToMultiByte (CP_UTF8, 0, str, -1, lstr, nLen, NULL, NULL);
	return lstr;
}






int GetWolfFileVersion (CString Wolf_Full_Path) 
{
	wchar_t *tp_path = (wchar_t*)(LPCTSTR)(Wolf_Full_Path);
	HANDLE hPath = CreateFileW(tp_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hPath == INVALID_HANDLE_VALUE) { return -2; }
	//*파일이 없으면 -2를 반환한다

	if (GetFileSize (hPath, NULL) < sizeof(WOLF_BIG_HEADER)) { CloseHandle(hPath); return -1; }
	//*파일이 깨졌으면 -1을 반환한다

	unsigned int Magic_Num, Magic_bak;
	unsigned char *pnt = (unsigned char*)(&Magic_Num);
	ReadFile (hPath, &Magic_Num, (DWORD)sizeof(unsigned int), NULL, NULL);
	Magic_bak = Magic_Num;
	CloseHandle(hPath);
	//*매직 넘버 사이즈만큼만 읽기

	for (unsigned int i = 0;i < sizeof(unsigned int);i++) { pnt[i] ^= WOLF_KEY[i%KEY_SIZE]; }
	if (Magic_Num == WOLF_MAGIC) { return DXA_LOW; }
	Magic_Num = Magic_bak;

	for (unsigned int i = 0;i < sizeof(unsigned int);i++) { pnt[i] ^= WOLF_KEY2[i%KEY_SIZE]; }
	if (Magic_Num == WOLF_MAGIC) { return DXA_220; }
	Magic_Num = Magic_bak;

	for (unsigned int i = 0;i < sizeof(unsigned int);i++) { pnt[i] ^= WOLF_KEY3[i%KEY_SIZE]; }
	if (Magic_Num == WOLF_MAGIC_64) { return DXA_330; }
	Magic_Num = Magic_bak;

	for (unsigned int i = 0;i < sizeof(unsigned int);i++) { pnt[i] ^= THMK2_KEY[i%KEY_SIZE]; }
	if (Magic_Num == DXA_MAGIC) { return DXA_THMK; }
	else if (Magic_Num == WOLF_MAGIC_64) { return DXA_THMK_64; }
	Magic_Num = Magic_bak;

	return -1;

	//*버전에 따라 DXA_LOW, DXA_220, DXA_330 중 하나를 반환한다. 어느 유형에도 해당되지 않으면 -1을 반환한다
}




bool CanBeFile (char *FileName, int code) 
{				
	int lp = Get_Last_SlashPos (FileName, code);
	int sl = (int)strlen (FileName);
	for (int ll = sl - 1;ll >= lp;ll--) {
		if (FileName[ll] == '?') { return false; }
	}
	return true;
	//*파일이 될 수 있는지 확인하여 반환한다. 물음표 있으면 안됨.
}




unsigned int Get_Total_FileCount (CString DirPath, CString Except) {
	WIN32_FIND_DATAW FData;
	HANDLE hFind;
	CString File_FullPath;
	unsigned int ret = 0;

	CString DirPath_ast = DirPath + _T("\\*");
	hFind = FindFirstFileW (DirPath_ast.GetBuffer(), &FData);
	do {
		File_FullPath = DirPath + _T("\\") + FData.cFileName;
		if (lstrcmp(FData.cFileName, Except.GetBuffer()) == 0){;}
		else if(!(FData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			&& (lstrcmp(FData.cFileName, L".") != 0) 
			&& (lstrcmp(FData.cFileName, L"..") != 0))
		{ ret++; }
		else if((lstrcmp(FData.cFileName, L".") != 0) && (lstrcmp(FData.cFileName, L"..") != 0)) {
			ret += Get_Total_FileCount(File_FullPath, Except);
		}
	}while (FindNextFileW (hFind, &FData));
	FindClose (hFind);

	return ret;
	//*재귀적으로 구하기, 파일이면 하나 더하고, 디렉토리면 그 디렉토리 내 총 파일 수 구해서 더하기
}




int Get_Text_Count_Or_Set_Text (unsigned char *buff, unsigned int buffsize, unsigned int code, FILE_TXT_ELEM *fte)
{
	unsigned char *BuffPnt = buff;
	unsigned int tp_strlen;
	int ret = 0;
	bool IsZero;
	bool IsTextofLang;
	unsigned int IsOneEnter;				//*0xA만으로 개행을 나타낼 때 쓰이는 변수
	unsigned int i;

	while (BuffPnt < (buff + buffsize - sizeof(int))) {

		tp_strlen = *(unsigned int*)(BuffPnt);
		//*먼저 문자열 길이값 얻기

		if ((tp_strlen >= ((unsigned int)(buff + buffsize) - (unsigned int)(BuffPnt + sizeof(int))))
			|| !((tp_strlen > 2) && (tp_strlen <= MAX_STRLEN))) {
			BuffPnt++; continue;
		}
		//*문자열 길이가 남은 길이보다 작은가, 그리고 최대 길이보다 작은가
		//*작지 않다면 하나 더하고 다시 올린다. 1~2바이트(0~1바이트 + NULL)는 의미없음

		unsigned char *BuffPnt_sub = BuffPnt + sizeof(int);
		//*문자열이 있는 위치 지정

		IsZero = false; IsTextofLang = true;
		i = 0; IsOneEnter = 1;
		//*기본적으로 0xA로 개행처리를 한다 가정 (1)
		while (i < tp_strlen) {

			if (BuffPnt_sub[i] == 0) { 
				if (i != (tp_strlen-1)) { IsZero = true; }
				break; 
			}
			//*맨 마지막 바이트에서 0이 아니라면 바로 나가리
			
			if (IS_ASCII (&BuffPnt_sub[i])) { i++; }
			else if (IS_ENTER (&BuffPnt_sub[i])) { 
				if (BuffPnt_sub[i+1] == 0xD) { IsOneEnter = 2; }
				//*0xD, 0xD면 2로 지정한다
				else if (BuffPnt_sub[i+1] == 0xA) { IsOneEnter = 0; }
				//*0xD, 0xA가 연이어 있을 때만 0으로 지정한다
				i += 2; 
			}
			else {

				if (code == JAP_CODE) {
					if (IS_SJIS (&BuffPnt_sub[i])) { i += 2; }
					else if (IS_NARROW_SJIS (&BuffPnt_sub[i])) { i++; }
					else { IsTextofLang = false; break; }
				}
				else if (code == KOR_CODE) {
					if (IS_CP949 (&BuffPnt_sub[i])) { i += 2; }
					else { IsTextofLang = false; break; }
				}

			}
			//***언어코드에 따라 해당 언어코드 텍스트인지 아닌지 여부를 판단한다***

			if (i >= tp_strlen) { IsTextofLang = false; break; }
			//*i가 tp_strlen을 넘어가면 바로 나가리

		}

		if (IsZero || !IsTextofLang) { BuffPnt++; continue; }
		//*맨 마지막 바이트 전 길이 체크 중에 0이 있어서는 안 되고, 해당 언어의 텍스트가 아니라도 안 된다.

		if (BuffPnt_sub[tp_strlen-1] != 0) { BuffPnt++; continue; }
		//*맨 마지막 바이트는 반드시 0이어야 한다.

		if (tp_strlen == 4) {
			if ((BuffPnt_sub[1] == 0x54) && (BuffPnt_sub[2] == 0x89)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[1] == 0xC6) && (BuffPnt_sub[2] == 0x2D)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[0] == 0x92) && (BuffPnt_sub[1] == 0x8E)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[0] == 0x8C) && (BuffPnt_sub[1] == 0xB9)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[0] == 0xA1) && (BuffPnt_sub[1] == 0x25)) { BuffPnt++; continue; }
		}
		else if (tp_strlen == 3) {
			if ((BuffPnt_sub[0] == 0xD) && (BuffPnt_sub[1] == 0xA)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[0] == 0x6A) && (BuffPnt_sub[1] == 0x42)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[0] == 0x84) && (BuffPnt_sub[1] == 0xAC)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[0] == 0xC8) && (BuffPnt_sub[1] == 0x32)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[0] == 0xCC) && (BuffPnt_sub[1] == 0x29)) { BuffPnt++; continue; }
			else if ((BuffPnt_sub[0] == 0xFA) && (BuffPnt_sub[1] == 0x63)) { BuffPnt++; continue; }
		}
		//***마지막 필터링을 시행한다. 조건까지 다 일치하지만 이건 걸리면 안되는 텍스트들이다.

		if (fte != NULL) {
			fte->Text_Idx_Arr[ret].Offset = (unsigned int)(BuffPnt - buff);
			fte->Text_Idx_Arr[ret].Length = tp_strlen;
			fte->Text_Idx_Arr[ret].Lang_Code = code;
			fte->Text_Idx_Arr[ret].File_Str_Idx = LOADING_FILE;
			fte->Text_Idx_Arr[ret].Is_OneChar_Enter = IsOneEnter;
		}
		//*FILE_TXT_ELEM 포인터의 값이 NULL이 아니라면 오프셋 값과 문자열 길이, 코드를 그대로 세팅한다
		//*파일 문자열 인덱스에는 아직 파일 문자열인지 뭔지 모르므로 로딩 중인 의미의 변수를 부여한다

		ret++;
		BuffPnt += (sizeof(int) + tp_strlen);
		//*무사히 통과했다면 텍스트로 인정하고 계속 넘긴다
	}
	//*루프를 계속 돌리면서 개수를 센다(매크로 이용)
	//*문자열 구조 : [길이값] + 문자열(null 포함)

	return ret;
}




unsigned int Get_Dir_Num_Or_DirList (CString Root_Dir, TCHAR **Dir_List) 
{
	WIN32_FIND_DATAW FData;
	HANDLE hFind;
	CString File_FullPath;
	unsigned int ret = 0;

	CString DirPath_ast = Root_Dir + _T("\\*");
	hFind = FindFirstFileW (DirPath_ast.GetBuffer(), &FData);
	do {
		File_FullPath = Root_Dir + _T("\\") + FData.cFileName;
		if((FData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			&& (lstrcmp(FData.cFileName, L".") != 0) 
			&& (lstrcmp(FData.cFileName, L"..") != 0))
		{ 
			ret++; 
			if (Dir_List != NULL) { 
				for (unsigned int i = 0;;i++){ 
					if (wcslen(Dir_List[i]) == 0) { 
						memcpy (Dir_List[i], File_FullPath.GetBuffer(), (sizeof(TCHAR)*File_FullPath.GetLength())); 
						for (unsigned int j = 0;j < wcslen(Dir_List[i]);j++) { if (Dir_List[i][j] == '\\') { Dir_List[i][j] = '/'; } }
						Dir_List[i][wcslen(Dir_List[i])] = '/';
						break; 
					}
				}
			}
			//*문자로 받을 게 있으면 디렉토리명을 넣어준다, 끝에는 추가로 '/' 하나 더 달아준다. 그래야 이상한 문자열이 안 바뀜
			ret += Get_Dir_Num_Or_DirList (File_FullPath, Dir_List); 
		}
	}while (FindNextFileW (hFind, &FData));
	FindClose (hFind);

	return ret;
}