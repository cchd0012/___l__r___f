#include "Wolf_Functions.h"

#include <curl/curl.h>
#include <CkCrypt2.h>

#include <iostream>
#include <sstream>
#include <cstdio>
#include "../resource.h"
#include "../Autotrans_Papago_Dialog.h"

#include "Wolf_Functions.h"

#include <cstring>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <iomanip>
#include <ctime>
#include <cmath>

void translateInit();
//*초기화 함수

void translateTerminate();
//*종결 함수

void Papago_Translate (Autotrans_Papago_Dialog *TransDlg, TCHAR *Translating_File, TCHAR *srcLang, TCHAR *destLang, unsigned int start_idx, unsigned int end_idx);
//*실제 번역 함수