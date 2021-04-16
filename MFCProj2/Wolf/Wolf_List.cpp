#include "stdafx.h"

#include "Wolf_List.h"
#include "Wolf_Functions.h"

extern CString Wolf_File_ListName;
extern CString Wolf_File_List;

unsigned int RootDir_StartPos;
//*진짜 문자열이 시작되는 곳

void FilePathCollect (wchar_t *DirPath, int Pos)
{
	HANDLE hFind;						//디렉토리 내 모든 파일을 찾는 데 쓰일 핸들
	WIN32_FIND_DATAW FData;				//찾아낸 파일의 정보를 가질 변수
	wchar_t ListPath[MAX_PATH];			//새 리스트 파일의 경로
	wchar_t PathChecker[MAX_PATH];		//기존 파일 이름과 새 파일 이름
	wchar_t PathChecker_fc[MAX_PATH];	//Pathchecker를 파일명과 결합하기 위한 변수
	wchar_t *EachFDName;				//각 파일/디렉토리 소유 이름
	wchar_t **FDStrings;				//파일/디렉토리 문자열 배열
	bool *DirFlags;						//문자열이 디렉토리인지 표시해주는 플래그 배열

	unsigned int NumOfFD;				//현재 디렉토리가 소유한 파일/디렉토리 총 수

	wchar_t EachListLine[MAX_STRLEN];	//리스트의 각 라인
	unsigned int FirstSlashPoint;		//문자열의 첫째 슬래시 위치를 담당한다
				
	memset (ListPath, 0, sizeof(wchar_t) * MAX_PATH);
	swprintf_s (ListPath, MAX_PATH, L"%s.txt", DirPath);

	if (Pos == 0) {
		RootDir_StartPos = 0;
		for (int i = (int)wcslen(DirPath) - 1;i >= 0;i--) {
			if ((DirPath[i] == '\\') || (DirPath[i] == '/')) { RootDir_StartPos = i+1; break; }
		}
		//*여기서 미리 계산해놓는다

		Wolf_File_ListName = &DirPath[RootDir_StartPos];
		Wolf_File_ListName += _T(".txt");
		RootDir_StartPos = 0;
		//*리스트명 갱신

		memset (EachListLine, 0, sizeof(wchar_t) * MAX_PATH);
		swprintf_s (EachListLine, MAX_PATH, L"%s\r\n", &DirPath[RootDir_StartPos]);
		Wolf_File_List += EachListLine;
		//*리스트 기록
	}
	else {
		memset (EachListLine, 0, sizeof(wchar_t) * MAX_PATH);

		for (FirstSlashPoint = 0;FirstSlashPoint < wcslen(DirPath);FirstSlashPoint++) {
			if ((DirPath[FirstSlashPoint] == '\\') || (DirPath[FirstSlashPoint] == '/')) { break; }
		}
		//*첫째 슬래시 지점 변수 초기화 및 계산.

		swprintf_s (EachListLine, MAX_PATH, L"\r\n%s\r\n", &DirPath[RootDir_StartPos]);
		Wolf_File_List += EachListLine;
		//*리스트 기록
	}
	//*만일 리스트 파일 핸들이 없다면, 경로 만들고 파일 핸들 형성
	//*또한 맨 처음이라는 소리이므로 (ROOT)0D0A를 버퍼에 기록하고 출력한다
	//*그리고 여기서 닫아야 하므로 IsRootFunction를 true로 한다
	//*만일 리스트 핸들이 있다면, 이미 진행중이라는 소리므로 DirPath를 그대로 박아넣는다

	memset (PathChecker, 0, sizeof(wchar_t) * MAX_PATH);
	memcpy (PathChecker, DirPath, sizeof(wchar_t) * wcslen(DirPath));
	wcscat_s (PathChecker, (wcslen(PathChecker) + 3), L"\\*");
	//[디렉토리]\*로 지정하면 하위 모든 파일을 찾을 수 있다.
	//".???" 식으로 하면 물음표의 수에 따라 확장자를 구분할 수 있는듯
	//*모든 파일을 찾기 위한 패턴 지정
	
	memset (PathChecker_fc, 0, sizeof(wchar_t) * MAX_PATH);
	memcpy (PathChecker_fc, DirPath, sizeof(wchar_t) * wcslen(DirPath));
	wcscat_s (PathChecker_fc, (wcslen(PathChecker_fc) + 3), L"\\");
	//*파일 결합용으로 하나 더 만들어놓기

	NumOfFD = 0;
	hFind = FindFirstFileW (PathChecker, &FData);	//*첫 파일 찾아두기
	do {
		if((lstrcmp(FData.cFileName, L".") != 0) && (lstrcmp(FData.cFileName, L"..") != 0)){ NumOfFD++; } //., .. 전부 제외
	} while (FindNextFileW (hFind, &FData));
	FindClose (hFind);
	//*파일/디렉토리 수 확정

	if (NumOfFD == 0) { 
		memset (EachListLine, 0, sizeof(wchar_t) * MAX_PATH);
		swprintf_s (EachListLine, MAX_PATH, L"\r\n");
		Wolf_File_List += EachListLine;
		return; 
		//*없으면 개행 한줄 치고 그냥 리턴한다
	}
	else {
		FDStrings = (wchar_t**)malloc(sizeof(wchar_t*)*NumOfFD);
		DirFlags = (bool*)malloc(sizeof(bool)*NumOfFD);
		for (unsigned int i = 0;i < NumOfFD;i++) {
			FDStrings[i] = (wchar_t*)malloc(sizeof(wchar_t)*MAX_PATH);
			memset (FDStrings[i], 0, sizeof(wchar_t) * MAX_PATH);
			DirFlags[i] = false;
		}
		//*확정한 수를 기준으로 파일/디렉토리 문자열 배열, 디렉토리 플래그 배열 형성(기본 false).
	}

	for (unsigned int i = 0;i < NumOfFD;i++) {
		if (i == 0) { hFind = FindFirstFileW (PathChecker, &FData); }		//*첫 파일/디렉토리 찾아두기
		else { FindNextFileW (hFind, &FData); }								//*다음 파일/디렉토리 찾아두기

		do {
			if((lstrcmp(FData.cFileName, L".") != 0) && (lstrcmp(FData.cFileName, L"..") != 0)){ break; }
		} while (FindNextFileW (hFind, &FData));
		//* '.', '..'이 있나 확인하고, 없을 때까지 돌린다

		EachFDName = FData.cFileName;

		if (FData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { DirFlags[i] = true; }
		//*디렉토리 플래그 체크

		for (FirstSlashPoint = (unsigned int)wcslen(PathChecker_fc);FirstSlashPoint < (unsigned int)wcslen(PathChecker_fc);FirstSlashPoint++) {
			if ((PathChecker_fc[FirstSlashPoint] == '\\') || (PathChecker_fc[FirstSlashPoint] == '/')) { break; }
		}
		//*첫째 슬래시 지점 변수 초기화 및 계산.

		swprintf_s (FDStrings[i], MAX_PATH, L"%s%s", PathChecker_fc, EachFDName);
		//*파일/디렉토리 문자열 만들기

		memset (EachListLine, 0, sizeof(wchar_t) * MAX_PATH);
		swprintf_s (EachListLine, MAX_PATH, L"\t%s\\%s\r\n", &DirPath[RootDir_StartPos], EachFDName);
		Wolf_File_List += EachListLine;
		//*문자열들 기록. 처음에 시작한 것과 합치고, 맨 앞의 슬래시를 찾아서 앞부분을 없애서 줄인다
		//*즉 "\t%s\\%s\r\n", 첫번째는 첫째 슬래시까지 가고 난 후의 PathChecker, 둘째는 위 파일이름의 일본어/한글변환

	}
	//*먼저 현재 기입된 경로 내에 있는 디렉토리/파일들부터 전부 처리한 후,

	for (unsigned int i = 0;i < NumOfFD;i++) {
		if (DirFlags[i]){ FilePathCollect (FDStrings[i], (Pos+1)); }
		//*디렉토리인 것들을 골라 순차적 재귀로 진행한다
		free (FDStrings[i]);
		//*그리고 이 작업을 마친 후 그 문자열을 깔끔하게 해제한다
	}
	//*디렉토리 순회 작업

	free (FDStrings); free (DirFlags);

	if (Pos == 0) { 
		memset (EachListLine, 0, sizeof(wchar_t) * MAX_PATH);
		swprintf_s (EachListLine, MAX_PATH, L"\r\n");
		Wolf_File_List += EachListLine;
	}
	//*디렉토리 배열, 문자열 배열, 리스트 핸들, 디렉토리 문자열 해제. 마지막으로 개행 하나 더 박고 끝낸다
}//파일 경로를 모아 새로운 리스트 파일을 만드는 함수