#include <atlstr.h>
#include "Wolf_Structure.h"

#ifndef FUNCTIONS
#define FUNCTIONS

#define PROGRESSVALUE(a, b) (int)((double)( (a) * 100 ) / (double) (b) )

#define MAX_STRLEN 0x400
#define MAX_INTLEN 0x10

#define ENTER_CHAR 0x00A7

#define FILESTRMAX 0x100

#define DEFAULT 0
#define WOLF 0x10
#define PAK 0x20
#define DAT 0x30
#define MED 0x40

#define DXA_LOW		0x1
#define DXA_220		0x2
#define DXA_330		0x3
#define DXA_THMK	0x4
#define DXA_THMK_64	0x5
//*울프툴 버전

#define JAP_CODE 0x0
#define KOR_CODE 0x1
#define NON_CODE 0xFFFFFFFF
//*언어 코드

#define GAMEDAT_HEADLEN 0xA

#define KEY_SIZE 0xC

#define BETWEEN(val, lo, hi) (((lo) <= (val)) && ((val) <= (hi)))
#define IS_ASCII(a) (BETWEEN(((a)[0]),0x20,0x7E) || (((a)[0]) == 0xA))
//*전형적인 ascii 문자인가
#define IS_ENTER(a) ((((a)[0]) == 0xD) && (((a)[1]) == 0xA)) || ((((a)[0]) == 0xD) && (((a)[1]) == 0xD))
//*개행 문자인가
#define IS_SJIS(a) ((BETWEEN(((a)[0]),0x81,0x9F)) || ((BETWEEN(((a)[0]),0xE0,0xEF)))) && ((BETWEEN(((a)[1]),0x40,0xFC)) && ((a)[1] != 0x7F))
//*전각 shift-jis인가
#define IS_NARROW_SJIS(a) BETWEEN(((a)[0]),0xA1,0xDF)
//*반각 shift-jis인가
#define IS_CP949(a) ((BETWEEN(((a)[0]),0x81,0xC5)) \
	&& ((BETWEEN(((a)[1]),0x41,0x5A)) || (BETWEEN(((a)[1]),0x61,0x7A)) || (BETWEEN(((a)[1]),0x81,0xFE)))) \
	|| ((((a)[0]) == 0xC6) && ((BETWEEN(((a)[1]),0x41,0x52)) || (BETWEEN(((a)[1]),0xA1,0xFE)))) \
	|| ((BETWEEN(((a)[0]),0xC7,0xFE)) && (BETWEEN(((a)[1]),0xA1,0xFE)))
//*CP949 문자인가
#define IS_NOFILE_CHAR(a) (((a) < 0x20) || ((a) == '/') || ((a) == '\\') \
	 || ((a) == '?') || ((a) == '*') || ((a) == ':') || ((a) == '"') \
	 || ((a) == '<') || ((a) == '>'))
//*금칙 제어 문자인가



static unsigned char WOLF_KEY[KEY_SIZE] = {0xF0, 0x35, 0x6B, 0x1C, 0xFB, 0x9B, 0xED, 0x1D, 0x03, 0x70, 0xE3, 0x2D};		//.wolf
static unsigned char WOLF_KEY2[KEY_SIZE] = {0xB3, 0x9D, 0xA0, 0x84, 0xD7, 0x37, 0x53, 0x1F, 0xF1, 0x08, 0x18, 0x80};	//.wolf개량
static unsigned char WOLF_KEY3[KEY_SIZE] = {0xC7, 0x05, 0xCA, 0x7D, 0x8D, 0xE3, 0xDE, 0xF1, 0xD9, 0x0C, 0x85, 0xF4};	//.wolf 64비트
static unsigned char THMK2_KEY[KEY_SIZE] = {0x95, 0xF6, 0xEB, 0xA9, 0xB9, 0xED, 0xBD, 0xB7, 0x0B, 0x19, 0x80, 0xA3};	//.동방의 미궁 2

static unsigned char GAMEDAT_HEAD[GAMEDAT_HEADLEN] = {0x00, 0x57, 0x00, 0x00, 0x4F, 0x4C, 0x00, 0x46, 0x4D, 0x00};

int My_mkdir (char *Dir_buffer, unsigned int code);

int FD_Rename (wchar_t *OldFileName, wchar_t *NewFileName);

void Converting_Big(char *dest, char *source, unsigned int length, unsigned int code);

int Get_Last_SlashPos(char *pathname, unsigned int code);

unsigned int Get_File_Count(unsigned char *filenam_tok, unsigned int filenam_tok_size);

void Set_FileNames(FILESTR_DATA *filenam_coll, unsigned int filenam_coll_size, unsigned char *filenam_tok);

int Is_File_Or_Dir (wchar_t* ss);

unsigned int Find_FileOrDirName_Idx(FILESTR_DATA *fsd, unsigned int fsd_size, unsigned int pos);

void Low_GameDat_Xor (unsigned char *Buff, unsigned int BuffSize);

wchar_t* J2U (char *string);

wchar_t* K2U (char *string);

char* U2K (wchar_t *string);

char* U2J (wchar_t *string);

wchar_t* UTF8_To_UTF16 (char *str);

char* UTF16_To_UTF8 (wchar_t *str);

int GetWolfFileVersion (CString Wolf_Full_Path);

bool CanBeFile (char *FileName, int code);

unsigned int Get_Total_FileCount (CString DirPath, CString Except);
//*해당 디렉토리 안에 있는 전체 파일 수를 구하는 함수. Except 문자열이 들어가 있으면 제외한다

int Get_Text_Count_Or_Set_Text (unsigned char *buff, unsigned int buffsize, unsigned int code, FILE_TXT_ELEM *fte);
//*해당 버퍼 안에 있는 해당 언어코드 텍스트 데이터의 수를 반환
//*파일당 텍스트 구조체 포인터의 값이 NULL이 아니면 같이 세팅해준다

unsigned int Get_Dir_Num_Or_DirList (CString Root_Dir, TCHAR **Dir_List);
//*Root_Dir 안에 있는 총 디렉토리 수를 반환하거나 Dir_List에 세팅하는 함수

#endif