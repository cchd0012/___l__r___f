#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <Windows.h>
#include <locale.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#ifndef STRUCTURE
#define STRUCTURE

#define DIR 0x10
#define FILE_DB 0x20
#define DB 0x26	
#define MOV 0x2026						//이 네 값들은 각각 디렉토리인지, 파일인지, 혹은 데이터베이스(.db)인지, 영상인지 판단하는 척도로 쓰일 것이다.
#define FILESTRMAX_T 0x100

#define NONE_FILE		0xFFFFFFFF		//파일 인덱스 지정 시 파일이 아닐 때의 값
#define LOADING_FILE	0x7FFFFFFF		//파일 리스트로 로딩 중일 때의 값

#define WOLF_MAGIC		0x00035844		//this should be the magic header of .wolf file. like 'DX♥'
#define DXA_MAGIC		0x00045844		//this should be the magic header of .dxa file. like 'DX♥'
#define WOLF_MAGIC_64	0x00065844		//this should be the magic header of .wolf file. like 'DX♥'
#define DXA_MAGIC_64	WOLF_MAGIC_64	//this should be the magic header of .dxa file. like 'DX♥'
#define HEAD_END		0x3A4			//maybe notifying that this is header end.
#define HEAD_END2		0x3B5			//maybe notifying that this is header end.
#define PADD			0xFFFFFFFF		//메타데이터 시 패딩값




typedef struct file_info_big{
#pragma pack(push,1)
	unsigned int fltok_start;	//파일 토큰에서 하나의 문자 더미가 시작하는 위치였음ㅠ
	unsigned int attr;			//this presents whether it's DIR or FILE.
	time_t generated;			//generated time
	time_t last_modified;		//last modified time
	time_t last_accessed;		//last access time
	unsigned int offset;		//file's offset, if it's directory, this will get value of 0x10's multiple.
	unsigned int UncompSize;	//file's size.
	unsigned int CompSize;		//압축된 크기
#pragma pack(pop)
}FILE_METADATA;
//***32비트 파일 메타데이터

typedef struct file_biginfo{
#pragma pack(push,1)
	__int64 fltok_start;	//파일 토큰에서 하나의 문자 더미가 시작하는 위치였음ㅠ
	__int64 attr;			//this presents whether it's DIR or FILE.
	time_t generated;		//generated time
	time_t last_modified;	//last modified time
	time_t last_accessed;	//last access time
	__int64 offset;			//file's offset, if it's directory, this will get value of 0x10's multiple.
	__int64 UncompSize;		//압축푼 파일 크기
	__int64 CompSize;		//압축된 파일 크기, 만일 압축이 안 된 파일이라면 -1(0xFFFFFFFFFFFFFFFF)이어야 한다
#pragma pack(pop)
}FILE_BIG_METADATA;
//***64비트 파일 메타데이터




typedef struct directory_idx{
#pragma pack(push,1)
	unsigned int dirmt_ofs;		//해당 디렉토리의 메타데이터가 위치한 곳(사실 메타데이터의 크기만큼 나눠야 함).
	int parent_dir;				//-1(0xFFFFFFFF) if dir is root directory, 0 if parent is root directory, 0x10, 0x20...means that sub dir's order.
	unsigned int mt_number;		//디렉토리가 가진 메타데이터의 갯수.
	unsigned int mt_offset;		//해당 디렉토리가 소유하는 메타데이터들이 위치한 곳(사실 메타데이터의 크기만큼 나눠야 함).
#pragma pack(pop)
}DIR_IDX;
//***32비트 전용 디렉토리 인덱스

typedef struct directory_idx64{
#pragma pack(push,1)
	__int64 dirmt_ofs;		//해당 디렉토리의 메타데이터가 위치한 곳(사실 메타데이터의 크기만큼 나눠야 함).
	__int64 parent_dir;		//-1(0xFFFFFFFF) if dir is root directory, 0 if parent is root directory, 0x10, 0x20...means that sub dir's order.
	__int64 mt_number;		//디렉토리가 가진 메타데이터의 갯수.
	__int64 mt_offset;		//해당 디렉토리가 소유하는 메타데이터들이 위치한 곳(사실 메타데이터의 크기만큼 나눠야 함).
#pragma pack(pop)
}DIR_BIG_IDX;
//***64비트 전용 디렉토리 인덱스




typedef struct filenm{
#pragma pack(push,1)
	unsigned int start;			//파일 토크에서 시작지점을 저장하기 위한 방법
	unsigned short len;			//(len*4) means least need length to save filename. so if 'sdfs.pl' length is 7 -> len = (8/4) = 2
	unsigned short chksum;		//this should be the sum of each filename alphabet's ascii code. check string is big_str.
	char *big_str;				//this means big alphabet's conversion of filename. ex)"BIGDATA.TXT"
	char *real_str;				//this means real(original) filename. ex)"Bigdata.txt"
#pragma pack(pop)
}FILESTR_DATA;




typedef struct ddrt{
	char file_Str[FILESTRMAX_T];
	unsigned int offset;
}FN_OFS_BUNCH;




typedef struct wolf_head{
#pragma pack(push,1)
	unsigned int magic;			//wolf 파일의 매직값이 된다(0x35844)
	unsigned int total_toklen;	//this should be (DIR_IDX's chunk's len)+(FILESTR_DATA's chunk's len)
	unsigned int header_size;	//Dxa header size.(should be 0x18)
	unsigned int totalmt_ofs;	//this means that start of total metadata(filename's chunk+file metadata's chunk+dir metadata's chunk)
	unsigned int filemt_ofs;	//this means that start of file metadata chunk's. so this is also means filename token's size
	unsigned int dir_hd_ofs;	//this means that start of DIR_IDX's chunks. so this is also (total_toklen - DIR_IDX's chunk lennth)
#pragma pack(pop)
}WOLF_HEADER;
//***32비트 전용 울프파일 헤더

typedef struct wolf_head2{
#pragma pack(push,1)
	unsigned int magic;			//서브 wolf 파일의 매직값이 된다(0x65844)
	unsigned int total_toklen;	//this should be (DIR_IDX's chunk's len)+(FILESTR_DATA's chunk's len)
	__int64 header_size;		//Dxa header size.(should be 0x30)
	__int64 totalmt_ofs;		//this means that start of total metadata(filename's chunk+file metadata's chunk+dir metadata's chunk)
	__int64 filemt_ofs;			//this means that start of file metadata chunk's. so this is also means filename token's size
	__int64 dir_hd_ofs;			//this means that start of DIR_IDX's chunks. so this is also (total_toklen - DIR_IDX's chunk lennth)
	__int64 end;				//this should be header end value(0x3A4).
#pragma pack(pop)
}WOLF_BIG_HEADER;
//***64비트 전용 울프파일 헤더




typedef struct Dir_Cnt_String{
#pragma pack(push,1)
	char *Dir_Str;					//디렉토리의 문자열
	unsigned int Dir_Meta_Cnt;		//디렉토리가 가진 [파일/디렉토리]의 수
	char **FileOrDir_In_Dir_Arr;	//각 디렉토리가 가진 [파일/디렉토리]들을 가리키게 하는 문자열 포인터 배열
#pragma pack(pop)
}FOD_IN_DIR;
//***합성 시 임시로 디렉토리가 가진 메타데이터 수와 문자열 포인터(디렉토리명)을 기억할 필요가 있기에 선언하는 구조체




typedef struct text_idx {
	unsigned int Lang_Code;
	unsigned int Offset;
	unsigned int Length;
	unsigned int File_Str_Idx;
	unsigned int Is_OneChar_Enter;
	unsigned int Is_Text_Dirty;
}TEXT_IDX;
//*각 텍스트의 언어 코드 정보, 오프셋, 길이를 합친 구조체, 파일 문자열 인덱스, 
//*0xA만으로 개행을 하는가, 수정하고 저장을 했는가 안 했는가
//*문자열 길이는 유니코드가 아닌 원래 코드를 기준으로 계산한다
//***Dirty 계열 변수는 tmptxt에 저장되지 않는다
//*파일 수는 한 개로 가정할 수 없으니,최대 8개까지로 가정할까 생각중. 배열로 처리

typedef struct file_idx {
	TCHAR File_Name[MAX_PATH];
	TCHAR File_FullPath[MAX_PATH];
	unsigned int Idx_of_Text_Start;
	unsigned int Text_Cnt;
	unsigned int Is_FileTxt_Dirty;
	TEXT_IDX *Text_Idx_Arr;
}FILE_TXT_ELEM;
//*각 파일 원소의 구조체(파일 이름, 전체 파일 경로, 텍스트 리스트의 시작 인덱스, 텍스트 구조체의 수, 
//*                  텍스트 구조체 포인터(할당용), 파일의 텍스트 데이터가 하나라도 수정 중인가)
//***Dirty 계열 변수는 tmptxt에 저장되지 않는다

typedef struct dir_file_txt {
	unsigned int File_Num;
	FILE_TXT_ELEM *File_Idx_Arr;
	unsigned int Is_Dirty;
}DIR_TXT_IDX;
//*한 디렉토리 내에 있는 모든 텍스트를 총괄하는 구조체.
//***Dirty 계열 변수는 tmptxt에 저장되지 않는다

#endif