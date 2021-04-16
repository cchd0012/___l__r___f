#include "../resource.h"
#include "../Load_Text_Dialog.h"
#include "../Save_Text_Dialog.h"
#include "../Change_FDName_Dialog.h"
#include "../Check_FDName_Dialog.h"

#include "Wolf_Functions.h"
#include "Wolf_Arclib.h"


int Load_Tmptxt (Load_Text_Dialog *LoadTtxDlg, CString DirPath);
//*해당 경로에서 tmptxt 파일을 불러와 출력하는 함수 (인자 : 프로그레스를 표시할 다이얼로그, 불러올 파일의 경로)
//*정상 로드 : 0 반환, 로드 실패 : -1 반환

void Save_Tmptxt (CString DirPath);
//*해당 경로에 tmptxt 파일을 저장하는 함수 (인자 : 불러올 파일 핸들, DIR_TXT_IDX 포인터), 체크할 필요 없음

void Load_Text_From_Directory_Data (Load_Text_Dialog *LoadTtxDlg, CString DirPath, unsigned int code, bool IsRoot);
//*디렉토리 내에 텍스트를 포함한 파일을 직접 로드해주는 함수

int Save_Text_To_Directory_Data (Save_Text_Dialog *SaveTxtDlg, CString DirPath);
//*수정된 텍스트를 디렉토리 내부 데이터에 저장해주는 함수

void Narrow_to_Em();
//*리스트에 있는 텍스트 중 반각을 전부 전각으로 바꿔주는 함수

void Check_Filename_For_Codepage (Check_FDName_Dialog *ChkDlg, unsigned int code);
//*해당 코드페이지에서 파일 이름이 깨지지 않는가, 깨지면 어떤 이름으로 바꿀 건가 정하도록 하는 함수

void Load_Hanja ();
//*Hanja.txt 파일을 로드하는 함수

void Release_Hanja ();
//*Hanja.txt 파일을 해제하는 함수

void JPHan_2_KRHan (CString *str);
//*바꿀 수 있는 일본한자를 모두 한국한자로 바꾸는 함수