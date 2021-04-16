#include "stdafx.h"

#include <map>
#include "Wolf_Auto_Papago_Translate.h"

#pragma warning (disable : 4996)

using namespace std;

struct MemoryStruct {
	char *memory;
	size_t size;
	size_t capacity;
};

#define BAD	 -1
#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)
#define UNIT_LENGTH 0x400

#define FORMDATA_KEY_VALUE_COUNT 10

static const char base64digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char uuidCharacters[] = "abcdefghijklmnopqrstuvwxyz0123456789";
char base64val[128];
static CkCrypt2 crypt;
static const char *papago_domain = "https://papago.naver.com/apis/n2mt/translate";
//*필요한 변수 및 정의들

static const char* postParams [FORMDATA_KEY_VALUE_COUNT * 2] = {
	"deviceId", NULL,
	"locale", "ko",
	"dict", "false",
	"dictDisplay", "30",
	"honorific", "false",
	"instant", "false",
	"paging", "false",
	"source", NULL,
	"target", NULL,
	"text", NULL
};
//*x-www-form-urlencoded 형식으로 넣어줄 key-value값들
//*첫 번째 NULL에는 랜덤 생성 UUID, 두 번째는 원래 텍스트의 언어(여기서는 "ja")
//*세 번째 NULL에는 번역된 텍스트의 언어(여기서는 "ko"), 네 번째 NULL은 UTF-8로 인코딩된 텍스트다

static CURL *curl;
//*libcurl을 이용하기 위한 변수

static struct MemoryStruct m_chunk;
//*데이터를 받을 변수


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

extern int Get_List_Index (unsigned int fidx, unsigned int tidx);


TCHAR *GetTranslatedText (TCHAR *srcLang, TCHAR *destLang, TCHAR *originText);
//*번역 수행 함수


void encode_base64(unsigned char *out, unsigned char *in, int inlen)
{
	for (; inlen >= 3; inlen -= 3)										//3바이트를 6비트 단위로 끊어서 4바이트로 만듬.
	{
		*out++ = base64digits[in[0] >> 2];								//0번째 앞6자리
		*out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];	//0번째 뒤2자리 | 1번째 앞4자리
		*out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];	//1번째 뒤4자리 | 2번째 앞2자리
		*out++ = base64digits[in[2] & 0x3f];							//2번째 뒤6자리
		in += 3;
	}

	if (inlen > 0)
	{
		unsigned char fragment;
		*out++ = base64digits[in[0] >> 2];
		fragment = (in[0] << 4) & 0x30;
		if (inlen > 1)  fragment |= in[1] >> 4;
		*out++ = base64digits[fragment];
		*out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
		*out++ = '=';
	}
	*out = 0;
}
//*base64 인코딩


static size_t WriteMemoryCallback (void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct*)userp;
	if (mem->capacity < (realsize + mem->size)) {
		while (mem->capacity < (realsize + mem->size)) {
			mem->capacity += UNIT_LENGTH;
		}
		char *new_m = (char*)malloc (mem->capacity);
		memcpy (new_m, mem->memory, mem->size);
		free (mem->memory); mem->memory = new_m;
	}
	// 메모리 크기를 엇나가면 보정한다
	if (mem->memory == NULL) {
		printf ("Memory allocate Error\n"); return 0;
	}
	memcpy (&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}
//*libcurl에서 기록 시 콜백 함수


int char2int (char input)
{
	if(input >= '0' && input <= '9'){ return input - '0'; }
	if(input >= 'A' && input <= 'F'){ return input - 'A' + 10; }
	if(input >= 'a' && input <= 'f'){ return input - 'a' + 10; }
	throw invalid_argument ("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void hex2bin (const char* src, char* target)
{
	while(*src && src[1])
	{
		*(target++) = char2int(*src)*16 + char2int(src[1]);
		src += 2;
	}
}

string hexStr(unsigned char* data, int len)
{
	stringstream ss;
	ss << hex;
	for(int i=0;i<len;++i) { ss << setw(2) << setfill('0') << (int)data[i]; }
	return ss.str();
}

char* stristr( const char* str1, const char* str2 )
{
	const char* p1 = str1;
	const char* p2 = str2;
	const char* r = (p2 == NULL) ? str1 : NULL;
	
	while( *p1 != 0 && *p2 != 0 )
	{
		if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
		{
			if( r == 0 ) { r = p1; }
			p2++ ;
		}
		else
		{
			p2 = str2 ;
			if ( r != 0 ) { p1 = r + 1 ; }
			if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2)) { r = p1 ; p2++ ; }
			else { r = 0 ; }
		}
		p1++ ;
	}
	return *p2 == 0 ? (char*)r : 0 ;
}
//*기타 기능들


void translateInit()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	crypt.put_EncodingMode("hex");
	crypt.put_HashAlgorithm("md5");
	crypt.SetHmacKeyEncoded("v1.5.2_0d13cb6cf4", "ansi");

	curl_easy_setopt(curl, CURLOPT_URL, "https://papago.naver.com/apis/n2mt/translate");
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	// url 지정 및 과정 출력 안함
		
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void*)&m_chunk);
	// 기록시 콜백 함수와 기록할 공간 지정

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	// 인증 무시
}
//*초기화 함수

void translateTerminate()
{
	curl_easy_cleanup (curl);
	curl_global_cleanup();
}
//*종결 함수



string getRandomUUID()
{
	ostringstream postBuf;
	for (unsigned int i = 0;i < 8;i++) { postBuf << uuidCharacters[rand() % 36]; }
	postBuf << '-';
	for (unsigned int i = 0;i < 4;i++) { postBuf << uuidCharacters[rand() % 36]; }
	postBuf << '-';
	for (unsigned int i = 0;i < 4;i++) { postBuf << uuidCharacters[rand() % 36]; }
	postBuf << '-';
	for (unsigned int i = 0;i < 4;i++) { postBuf << uuidCharacters[rand() % 36]; }
	postBuf << '-';
	for (unsigned int i = 0;i < 12;i++) { postBuf << uuidCharacters[rand() % 36]; }
	string ret_s = postBuf.str();
	postBuf << flush;
	return ret_s;
	// 8바이트 + '-' + 4바이트 + '-' + 4바이트 + '-' + 4바이트 + '-' + 12바이트
	// 허용 인덱스 : 26 ~ 61 (36개)
}
//*UUID 생성 함수


ULONGLONG getCurrentUCTTimeAsMillisecond()
{
	SYSTEMTIME systemTime;
	GetSystemTime (&systemTime);
	FILETIME fileTime;
	SystemTimeToFileTime (&systemTime, &fileTime);
	ULONGLONG fileTimeNano100;
	fileTimeNano100 = (((ULONGLONG) fileTime.dwHighDateTime) << 32) + fileTime.dwLowDateTime;
	ULONGLONG posixTime = fileTimeNano100/10000 - 11644473600000;
	return posixTime;
}
//*현재 UTC 시간 밀리초 반환 함수


void getEncodedToken (char *uuid, ULONGLONG millisecond, char *token_sav)
{
	char _t[100]; memset (_t, 0, 100); sprintf(_t, "%llu", millisecond);
	string cc = uuid; cc += "\n"; cc += papago_domain; cc += "\n"; cc += _t;
	const char *mac = crypt.hmacStringENC(cc.c_str());
	char hex[16]; memset (hex, 0, 16); hex2bin (mac, hex);
	char *st = (char*)malloc(24 + 1); memset (st, 0, 24 + 1);
	encode_base64 ((unsigned char*)token_sav, (unsigned char*)hex, 16);
}
//*암호화 토큰 반환 함수


void Papago_Translate (Autotrans_Papago_Dialog *TransDlg, TCHAR *Translating_File, TCHAR *srcLang, TCHAR *destLang, unsigned int start_idx, unsigned int end_idx)
{
	// 인덱스를 이용한 범위 지정도 생각해본다
	map<CString, CString> cachier;

	TransDlg->SetDlgItemText (IDC_TRANS_PROG_TEXT, _T("Translating Progress : "));
	CString fstr, tstr, apply_Txt;
	
	unsigned int total_idx = end_idx - start_idx + 1;

	unsigned int File_Idx = 0;
	for (unsigned int i = 0;i < m_Text_Idx_In_Dir.File_Num;i++) {
		if (_tcsstr (m_Text_Idx_In_Dir.File_Idx_Arr[i].File_FullPath, Translating_File) != NULL) {
			File_Idx = i; break;
		}
	}
	//*번역할 파일의 인덱스 구하기
		
	int T_List_Idx = Get_List_Index (File_Idx, 0);
	TransDlg->progressing_file_idx = File_Idx;
	//*정보 세팅

	for (unsigned int j = start_idx;j <= end_idx;j++) {

		CString tmpCStr; 
		tmpCStr.Format(_T("Text in File %s(%d/%d)"), Translating_File, j, m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Text_Cnt);
		TransDlg->SetDlgItemText (IDC_TRANS_PROG_TEXT, tmpCStr);
		//*현재 번역 상태 정보 띄우기

		TransDlg->m_Translate_Progress.SetPos (PROGRESSVALUE((j-start_idx), total_idx));
		//*프로그레스 바 세팅

		tstr = m_G_ListCtrl_Text->GetItemText (j + m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Idx_of_Text_Start + 1, 0);
		//*텍스트

		if (m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Text_Idx_Arr[j].File_Str_Idx == NONE_FILE) {
			CString srcLang, destLang;
			CString orgStr = _T(""), transStr = _T("");

			if (m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Text_Idx_Arr[j].Is_Text_Dirty) {
				orgStr = tstr.Right (tstr.GetLength() - Prefix_Length - Dirty_Prefix_Length);
			}
			else {
				orgStr = tstr.Right (tstr.GetLength() - Prefix_Length);
			}
			//*더티 문자열, prefix 문자열 제거 후 진행

			for (unsigned int t_rep = 0;t_rep < 3;t_rep++) {
				if (cachier.find(orgStr) == cachier.end()) {
					TransDlg->m_Src_Language_List.GetLBText (TransDlg->m_Src_Language_List.GetCurSel(), srcLang);
					TransDlg->m_Dst_Language_List.GetLBText (TransDlg->m_Dst_Language_List.GetCurSel(), destLang);
					TCHAR *trans_text = GetTranslatedText (srcLang.GetBuffer(), destLang.GetBuffer(), orgStr.GetBuffer());
					transStr = trans_text;
					free (trans_text);
					//*글자가 맵에 없으면 번역 (어차피 텍스트 수도 한정되어 있으니 이러는 게 나을 것 같다)
					cachier.insert(pair<CString, CString>(orgStr, transStr));
					//*번역 후 맵에 추가
				}
				// 맵에서 못 찾을 때
				else { transStr = cachier[orgStr]; }
				// 맵에서 찾을 때
				if (transStr.GetLength() != 0) { break; }
			}

			if (transStr.GetLength() == 0) {
				tmpCStr.Format (_T("%s:[%06d] in %s:번역 실패!"), orgStr, j, Translating_File);
				AfxMessageBox (tmpCStr);
			}
			//*만일 번역 실패하면 메시지창 띄우고 빠져나간다
			//*혹시 서버 장애로 안됐을 수도 있으니 3번까지 재시도해보는 것도 괜찮을 것 같다
			//*번역 실패한 텍스트 인덱스를 띄워줘서 뭘 직접 수정해야 알지 알려준다

			else {
				m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Text_Idx_Arr[j].Is_Text_Dirty = true;	
				apply_Txt.Format (_T("%s[%06d]%s%s"), 
					Prefix_Text_Str[m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Text_Idx_Arr[j].Lang_Code], 
					j, T_Head_Dirty, transStr);
				m_G_ListCtrl_Text->SetItemText ((T_List_Idx + j), 0, apply_Txt);
				//*번역 후에는 무조건 더티로 고정시키고 더티 문자열, prefix 다 붙여서 리스트에 반영한다
			}
		}
		//*오직 파일명이 아닐 때만 진행한다
	}

	if (!m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Is_FileTxt_Dirty) {
		apply_Txt = T_Head_Dirty; apply_Txt += m_G_ListCtrl_Text->GetItemText ((T_List_Idx - 1), 0);
		m_G_ListCtrl_Text->SetItemText ((T_List_Idx - 1), 0, apply_Txt);
		m_Text_Idx_In_Dir.File_Idx_Arr[File_Idx].Is_FileTxt_Dirty = true;
	}
	//*반각 문자가 있다면 파일 헤더명 Dirty 상태로 바꾸기
	//*캐시용으로 map을 만들고 있으면 거기서 골라 처리하도록 한다

	if (!m_Text_Idx_In_Dir.Is_Dirty) {
		m_Text_Idx_In_Dir.Is_Dirty = true;
	}
	//*파일, 원본에도 Dirty 상태 적용

	cachier.clear();
	TransDlg->m_Translate_Progress.SetPos (100);
	TransDlg->SetDlgItemText (IDC_TRANS_PROG_TEXT, _T("Translating Finished"));
	//*프로그레스 바 갱신
}
//*실제 번역 수행 함수


TCHAR *GetTranslatedText (TCHAR *srcLang, TCHAR *destLang, TCHAR *originText)
{
	char *utf8txt = UTF16_To_UTF8 (originText);
	// 번역할 텍스트 UTF-8로 변환

	struct curl_slist *chunk = NULL;
	// 포스트 필드용 변수

	m_chunk.memory = (char*)malloc (UNIT_LENGTH);
	memset (m_chunk.memory, 0, UNIT_LENGTH);
	m_chunk.capacity = UNIT_LENGTH;
	m_chunk.size = 0;
	// 미리 간단하게 할당

	curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void*)&m_chunk);
	// 기록시 콜백 함수와 기록할 공간 지정

	ULONGLONG t_set = getCurrentUCTTimeAsMillisecond();
	// 디바이스 아이디(uuid)와 현재시간
	// 디바이스 아이디는 무작위 지정가능하므로 실행할 때 한 번 생성하고 계속 써먹는다

	string deviceIdStr, millisecondStr, tokenStr;

	char _t[100];
	deviceIdStr = getRandomUUID();
	memset (_t, 0, 100); sprintf (_t, "%llu", t_set); millisecondStr = _t;
	memset (_t, 0, 100); getEncodedToken ((char*)deviceIdStr.c_str(), t_set, _t); tokenStr = _t;
	// 디바이스 아이디(uuid)와 현재 밀리초 구해두고 토큰 얻기
	// string 객체로 저장해두기

	postParams[1] = deviceIdStr.c_str();
	char src_lang[3], dest_lang[3];
	src_lang[0] = (char)srcLang[0]; src_lang[1] = (char)srcLang[1]; src_lang[2] = 0;
	dest_lang[0] = (char)destLang[0]; dest_lang[1] = (char)destLang[1]; dest_lang[2] = 0;
	postParams[15] = src_lang;
	postParams[17] = dest_lang;
	postParams[19] = utf8txt;
	// 디바이스 아이디, 원본 언어, 번역된 언어, 텍스트를 받아온 인자들로 지정
		
	ostringstream postBuf;
	for (unsigned int i = 0;i < FORMDATA_KEY_VALUE_COUNT;i++) {
		char* key = curl_escape (postParams[i*2], 0);
		char* val = curl_escape (postParams[i*2+1], 0);
		postBuf << key << "=" << val;
		if (i < (FORMDATA_KEY_VALUE_COUNT-1)) { postBuf << "&"; }
		curl_free (key); curl_free (val);
		// 영어가 아닌 문자들을 x-www-form-urlencoded 형식으로 바꿔치기
		// 이것도 엄연한 할당이므로 마지막에 해제시켜준다
	}
	// x-www-form-urlencoded 형식으로 기록하기

	string postData; postData = postBuf.str();
	postBuf << flush;
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	// form 데이터 세팅

	string author_Str = "PPG "; author_Str += deviceIdStr; author_Str += ":"; author_Str += tokenStr;
	// authorization은 따로 뽑아두기

	string __t;
	chunk = curl_slist_append(chunk, "Accept: application/json");
	chunk = curl_slist_append(chunk, "Accept-Language: ko");
	__t = "Authorization: "; __t += author_Str;
	chunk = curl_slist_append(chunk, __t.c_str());
	chunk = curl_slist_append(chunk, "Connection: keep-alive");
	memset (_t, 0, 100); sprintf (_t, "%d", postData.length());
	__t = "Content-Length: "; __t += _t;
	chunk = curl_slist_append(chunk, __t.c_str());
	chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	chunk = curl_slist_append(chunk, "device-type: pc");
	chunk = curl_slist_append(chunk, "Host: papago.naver.com");
	chunk = curl_slist_append(chunk, "Origin: https://papago.naver.com");
	chunk = curl_slist_append(chunk, "Referer: https://papago.naver.com/");
	chunk = curl_slist_append(chunk, "TE: Trailers");
	__t = "Timestamp: "; __t += millisecondStr;
	chunk = curl_slist_append(chunk, __t.c_str());
	chunk = curl_slist_append(chunk, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:79.0) Gecko/20100101 Firefox/79.0");
	chunk = curl_slist_append(chunk, "x-apigw-partnerid: papago");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	// 필요한 헤더 추가 후 세팅

	CURLcode rc = curl_easy_perform(curl);
	free (utf8txt);
	// 리퀘스트 전송
		
	if(CURLE_OK != rc){
//		cerr << "Error from cURL: " << curl_easy_strerror (rc) << endl;
		free (m_chunk.memory);
		return NULL;
	}
	// 에러가 있으면 메시지 띄우고 돌아감

	char *translatedText_h;
	if ((translatedText_h = stristr((const char*)m_chunk.memory, "\"translatedText\"")) != NULL) {
		char *translatedText = translatedText_h + strlen("\"translatedText\":\"");
		char *p = translatedText;
		while ((size_t)(p - m_chunk.memory) < m_chunk.size) { 
			if (*p == '"') {
				if (*(p-1) == '\\') { p++; continue; }
				else { break; }
			}
			else { p++; continue; }
		}
		// JSON에서는 큰따옴표를 '\"'로 쓰므로 그걸 고려하여 걸러낸다
		// \r\n 혹은 \n으로 번역되는 경우가 있는데 이럴때는 그냥 개행(0xD, 0xA / 0xA)으로 처리해야 될 것 같다
		// 거기다 원래 \가 하나만 있었는데 2개로 늘어나며 번역되는 경우가 생겼다
		// 일단 이것들은 보완해서 배포하기로 하자

		*p = 0;
		TCHAR *utf16txt = UTF8_To_UTF16 (translatedText);
		CString cs = utf16txt;
		cs.Replace (_T("\\r"), _T("\r"));
		cs.Replace (_T("\\n"), _T("\n"));
		cs.Replace (_T("\\\\"), _T("\\"));
		free (utf16txt);
		utf16txt = (TCHAR*)malloc (sizeof(TCHAR) * (cs.GetLength()+1));
		memcpy (utf16txt, cs.GetBuffer(), sizeof(TCHAR) * cs.GetLength());
		utf16txt[cs.GetLength()] = 0;

		curl_slist_free_all (chunk);
		free (m_chunk.memory);
		// 메모리 정리

		return utf16txt;
		// (이 때 translatedText가 바로 번역된 텍스트가 된다)
	}
	else {
		// 메시지를 띄운다

		curl_slist_free_all (chunk);
		free (m_chunk.memory);
		// 메모리 정리

		return NULL;
	}
	// translatedText에 있는 값만 빼오고 utf-8 -> utf-16으로 디코딩한다
	// 큰따옴표 안에 둘러쌓인 값만 빼오면 될듯
}
//*실질적인 번역 함수