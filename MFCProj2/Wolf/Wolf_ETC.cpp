#include "stdafx.h"

#include "Wolf_ETC.h"



void Exe_Change (wchar_t *GameExeName)
{
	HANDLE hExe;
	unsigned char *ExeFileBuff;
	unsigned int ExeFileSize;

	DWORD lp_read, lp_write;
	
	char *NewStr = "Game_New.exe";

	unsigned char Xor_Eax[6] = {0x34, 0x20, 0x04, 0x5F, 0x3C, 0x3C};
	unsigned char Xor_Ebx[9] = {0x80, 0xF3, 0x20, 0x80, 0xC3, 0x5F, 0x80, 0xFB, 0x3C};
	unsigned char Xor_Ecx[9] = {0x80, 0xF1, 0x20, 0x80, 0xC1, 0x5F, 0x80, 0xF9, 0x3C};
	unsigned char Xor_Edx[9] = {0x80, 0xF2, 0x20, 0x80, 0xC2, 0x5F, 0x80, 0xFA, 0x3C};
	//xor 바이트 패턴
	
	unsigned char NewXor_Eax[6] = {0x34, 0x80, 0x2C, 0x01, 0x3C, 0x7D};
	unsigned char NewXor_Ebx[9] = {0x80, 0xF3, 0x80, 0x80, 0xEB, 0x01, 0x80, 0xFB, 0x7D};
	unsigned char NewXor_Ecx[9] = {0x80, 0xF1, 0x80, 0x80, 0xE9, 0x01, 0x80, 0xF9, 0x7D};
	unsigned char NewXor_Edx[9] = {0x80, 0xF2, 0x80, 0x80, 0xEA, 0x01, 0x80, 0xFA, 0x7D};
	//바뀐 후의 xor 바이트 패턴

	unsigned char Cmp_Eax[5] = {0x3D, 0x9F, 0x00, 0x00, 0x00};
	unsigned char Cmp_Ebx[6] = {0x81, 0xFB, 0x9F, 0x00, 0x00, 0x00};
	unsigned char Cmp_Ecx[6] = {0x81, 0xF9, 0x9F, 0x00, 0x00, 0x00};
	unsigned char Cmp_Edx[6] = {0x81, 0xFA, 0x9F, 0x00, 0x00, 0x00};
	//cmp 바이트 패턴

	unsigned char NewCmp_Eax[5] = {0x3D, 0xFF, 0x00, 0x00, 0x00};
	unsigned char NewCmp_Ebx[6] = {0x81, 0xFB, 0xFF, 0x00, 0x00, 0x00};
	unsigned char NewCmp_Ecx[6] = {0x81, 0xF9, 0xFF, 0x00, 0x00, 0x00};
	unsigned char NewCmp_Edx[6] = {0x81, 0xFA, 0xFF, 0x00, 0x00, 0x00};
	//바뀐 후의 cmp 바이트 패턴

	unsigned char String1[0x3B] = { 
		0x8B, 0x4E, 0x93, 0xAE, 0x92, 0x86, 0x82, 0xC5, 
		0x82, 0xB7, 0x81, 0x41, 0x82, 0xB1, 0x82, 0xCC, 
		0x8F, 0x88, 0x97, 0x9D, 0x82, 0xC9, 0x82, 0xCD, 
		0x82, 0xB5, 0x82, 0xCE, 0x82, 0xE7, 0x82, 0xAD, 
		0x8E, 0x9E, 0x8A, 0xD4, 0x82, 0xAA, 0x82, 0xA9, 
		0x82, 0xA9, 0x82, 0xE9, 0x82, 0xB1, 0x82, 0xC6, 
		0x82, 0xAA, 0x82, 0xA0, 0x82, 0xE8, 0x82, 0xDC, 
		0x82, 0xB7, 0x00 
	};
	unsigned char String2[0x25] = {
		0x8B, 0x4E, 0x93, 0xAE, 0x92, 0x86, 0x82, 0xC5, 
		0x82, 0xB7, 0x81, 0x41, 0x8F, 0x89, 0x8A, 0xFA, 
		0x89, 0xBB, 0x8F, 0x88, 0x97, 0x9D, 0x82, 0xF0, 
		0x8D, 0x73, 0x82, 0xC1, 0x82, 0xC4, 0x82, 0xA2, 
		0x82, 0xDC, 0x82, 0xB7, 0x00
	};
	unsigned char String3[0x9] = { 0x89, 0xE6, 0x96, 0xCA, 0x95, 0xCF, 0x8D, 0x58, 0x00 };
	unsigned char String4[0x9] = { 0x8F, 0xEE, 0x95, 0xF1, 0x95, 0xCF, 0x8D, 0x58, 0x00 };
	unsigned char String5[0x9] = { 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x00 };
	unsigned char String6[0x28] = {
		0x5B, 0x95, 0x60, 0x89, 0xE6, 0x46, 0x50, 0x53, 
		0x25, 0x64, 0x2F, 0x93, 0xE0, 0x95, 0x94, 0x46, 
		0x50, 0x53, 0x25, 0x64, 0x5D, 0x20, 0x5B, 0x25, 
		0x73, 0x28, 0x46, 0x35, 0x82, 0xC5, 0x95, 0xCF, 
		0x8D, 0x58, 0x29, 0x5D, 0x81, 0x40, 0x20, 0x00
	};
	unsigned char String7[0x16] = {
		0x8D, 0x82, 0x83, 0x58, 0x83, 0x79, 0x83, 0x62, 
		0x83, 0x4E, 0x97, 0x70, 0x2F, 0x82, 0xC8, 0x82, 
		0xDF, 0x82, 0xE7, 0x82, 0xA9, 0x00
	};
	unsigned char String8[0x16] = {
		0x92, 0x86, 0x83, 0x58, 0x83, 0x79, 0x83, 0x62, 
		0x83, 0x4E, 0x97, 0x70, 0x2F, 0x83, 0x6D, 0x81, 
		0x5B, 0x83, 0x7D, 0x83, 0x8B, 0x00
	};
	unsigned char String9[0x16] = {
		0x92, 0xE1, 0x83, 0x58, 0x83, 0x79, 0x83, 0x62, 
		0x83, 0x4E, 0x97, 0x70, 0x2F, 0x83, 0x4B, 0x83, 
		0x4E, 0x83, 0x4B, 0x83, 0x4E, 0x00
	};

	char *Chg_String1 = "기동 중입니다. 이 처리는 다소 시간이 걸릴 수 있습니다.";
	char *Chg_String2 = "기동 중입니다. 초기화를 처리합니다.";
	char *Chg_String3 = "화면변경";
	char *Chg_String4 = "정보변경";
	char *Chg_String5 = "　　　　";
	char *Chg_String6 = "[주사FPS%d/내부FPS%d] [%s(F5로 변경)]  ";
	char *Chg_String7 = "고사양 전용/매끄럽게";
	char *Chg_String8 = "중간사양 전용/노멀";
	char *Chg_String9 = "저사양 전용/간당간당";
	//바뀐 후의 문자열


	bool checker;

	hExe = CreateFileW (GameExeName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hExe != INVALID_HANDLE_VALUE){

		ExeFileSize = GetFileSize (hExe, NULL);
		ExeFileBuff = (unsigned char*)malloc(sizeof(char)*ExeFileSize);
		ReadFile (hExe, ExeFileBuff, ExeFileSize, &lp_read, NULL);
		CloseHandle (hExe);

		for (unsigned int i = 0;i < (ExeFileSize-9);i++) {

			checker = true;
			for (unsigned int j = 0;j < 6;j++) {
				if (ExeFileBuff[i+j] != Xor_Eax[j]) { checker = false; break; }
			}
			if (checker) { for (unsigned int j = 0;j < 6;j++) { ExeFileBuff[i+j] = NewXor_Eax[j]; } i += 6; }

			checker = true;
			for (unsigned int j = 0;j < 9;j++) {
				if (ExeFileBuff[i+j] != Xor_Ebx[j]) { checker = false; break; }
			}
			if (checker) { for (unsigned int j = 0;j < 9;j++) { ExeFileBuff[i+j] = NewXor_Ebx[j]; } i += 9; }

			checker = true;
			for (unsigned int j = 0;j < 9;j++) {
				if (ExeFileBuff[i+j] != Xor_Ecx[j]) { checker = false; break; }
			}
			if (checker) { for (unsigned int j = 0;j < 9;j++) { ExeFileBuff[i+j] = NewXor_Ecx[j]; } i += 9; }

			checker = true;
			for (unsigned int j = 0;j < 9;j++) {
				if (ExeFileBuff[i+j] != Xor_Edx[j]) { checker = false; break; }
			}
			if (checker) { for (unsigned int j = 0;j < 9;j++) { ExeFileBuff[i+j] = NewXor_Edx[j]; } i += 9; }

		}
		//xor 패턴 바꾸기

		for (unsigned int i = 0;i < (ExeFileSize-6);i++) {

			checker = true;
			for (unsigned int j = 0;j < 5;j++) {
				if (ExeFileBuff[i+j] != Cmp_Eax[j]) { checker = false; break; }
			}
			if (checker) { for (unsigned int j = 0;j < 5;j++) { ExeFileBuff[i+j] = NewCmp_Eax[j]; } i += 5; }

			checker = true;
			for (unsigned int j = 0;j < 6;j++) {
				if (ExeFileBuff[i+j] != Cmp_Ebx[j]) { checker = false; break; }
			}
			if (checker) { for (unsigned int j = 0;j < 6;j++) { ExeFileBuff[i+j] = NewCmp_Ebx[j]; } i += 6; }

			checker = true;
			for (unsigned int j = 0;j < 6;j++) {
				if (ExeFileBuff[i+j] != Cmp_Ecx[j]) { checker = false; break; }
			}
			if (checker) { for (unsigned int j = 0;j < 6;j++) { ExeFileBuff[i+j] = NewCmp_Ecx[j]; } i += 6; }

			checker = true;
			for (unsigned int j = 0;j < 6;j++) {
				if (ExeFileBuff[i+j] != Cmp_Edx[j]) { checker = false; break; }
			}
			if (checker) { for (unsigned int j = 0;j < 6;j++) { ExeFileBuff[i+j] = NewCmp_Edx[j]; } i += 6; }

		}
		//cmp 패턴 바꾸기

		for (unsigned int i = 0;i < (ExeFileSize-0x40);i++) {

			checker = true;
			for (unsigned int j = 0;j < 0x3B;j++) {
				if (ExeFileBuff[i+j] != String1[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String1);j++) { ExeFileBuff[i+j] = Chg_String1[j]; } 
				ExeFileBuff[i+strlen(Chg_String1)] = 0x00;
				i += 0x3B; 
			}

			checker = true;
			for (unsigned int j = 0;j < 0x25;j++) {
				if (ExeFileBuff[i+j] != String2[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String2);j++) { ExeFileBuff[i+j] = Chg_String2[j]; } 
				ExeFileBuff[i+strlen(Chg_String2)] = 0x00;
				i += 0x25; 
			}

			checker = true;
			for (unsigned int j = 0;j < 0x8;j++) {
				if (ExeFileBuff[i+j] != String3[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String3);j++) { ExeFileBuff[i+j] = Chg_String3[j]; } 
				i += 0x8; 
			}

			checker = true;
			for (unsigned int j = 0;j < 0x8;j++) {
				if (ExeFileBuff[i+j] != String4[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String4);j++) { ExeFileBuff[i+j] = Chg_String4[j]; } 
				i += 0x8; 
			}

			checker = true;
			for (unsigned int j = 0;j < 0x8;j++) {
				if (ExeFileBuff[i+j] != String5[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String5);j++) { ExeFileBuff[i+j] = Chg_String5[j]; } 
				i += 0x8; 
			}

			checker = true;
			for (unsigned int j = 0;j < 0x27;j++) {
				if (ExeFileBuff[i+j] != String6[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String6);j++) { ExeFileBuff[i+j] = Chg_String6[j]; } 
				i += 0x27; 
			}

			checker = true;
			for (unsigned int j = 0;j < 0x16;j++) {
				if (ExeFileBuff[i+j] != String7[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String7);j++) { ExeFileBuff[i+j] = Chg_String7[j]; } 
				ExeFileBuff[i+strlen(Chg_String7)] = 0x00;
				i += 0x16; 
			}

			checker = true;
			for (unsigned int j = 0;j < 0x16;j++) {
				if (ExeFileBuff[i+j] != String8[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String8);j++) { ExeFileBuff[i+j] = Chg_String8[j]; } 
				ExeFileBuff[i+strlen(Chg_String8)] = 0x00;
				i += 0x16; 
			}

			checker = true;
			for (unsigned int j = 0;j < 0x16;j++) {
				if (ExeFileBuff[i+j] != String9[j]) { checker = false; break; }
			}
			if (checker) { 
				for (unsigned int j = 0;j < strlen(Chg_String9);j++) { ExeFileBuff[i+j] = Chg_String9[j]; } 
				ExeFileBuff[i+strlen(Chg_String9)] = 0x00;
				i += 0x16; 
			}

		}
		//문자열 바꾸기

		hExe = CreateFileA(NewStr, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile (hExe, ExeFileBuff, ExeFileSize, &lp_write, NULL);
		CloseHandle (hExe);
		//파일 재기록

		free (ExeFileBuff);
	}
	else { AfxMessageBox (_T("해당 파일이 없습니다")); }

}







void Edit_Game_Dat ()
{

	HANDLE hGameDat;
	unsigned char *GameDatBuff, *buffpnt, *New_GameDatBuff, *newbuffpnt;
	unsigned int GameDatSize, New_GameDatSize;
	bool LowGameDat = false;
	unsigned int Start_Offset;
	unsigned int code, check_size;
	unsigned int additional_bytes = 0;
	//*game.dat 파일을 편집할 때 쓸 변수들

	CString Game_Title, Font1, Font2, Font3, Font4, ttt;
	char *Zeros, *Spec, *imgstr, *version;
	unsigned int tp_strlen;
	char *tmp;
	wchar_t *tmpuni;
	//*game.dat의 변수들을 받을 때 쓸 값들

	//*game.dat을 편집하여 길이가 변경될 경우, [전체 크기 - 1]의 정수값이 담긴 바이트가 있다
	//*그 값을 [바뀐 크기 - 1]로 적용하면 해결 가능하다. 글꼴 다음다음에 있다.
	//*순서 : 게임이름 -> 0000-0000 -> 8P@(rO!p;s5(특수문자열) -> 글꼴 1,2,3,4
	//*      -> 다음(이미지 이름) -> 다음(버전) -> [전체 크기 - 1]
	//*처음부터 뒤져보는 게 나을 것 같다. Game.dat마다 차이가 좀 있음.
	//*만일 0x26부터 시작되면 type 1/2고 언어코드를 따질 필요가 없음, 
	//*0x28이면 type 3이고 0x22(0x1F)에서 언어코드를 따짐

	hGameDat = CreateFileW (L"Data/BasicData/Game.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hGameDat == INVALID_HANDLE_VALUE) {
		AfxMessageBox (_T("Game.dat 파일이 없습니다.")); return;
	}
	GameDatSize = GetFileSize (hGameDat, NULL);
	GameDatBuff = (unsigned char*)malloc (sizeof(char) * GameDatSize);
	ReadFile (hGameDat, GameDatBuff, GameDatSize, NULL, NULL);
	CloseHandle (hGameDat);
	//*game.dat 파일 불러오기

	for (unsigned int i = 0;i < GAMEDAT_HEADLEN;i++) {
		if (GameDatBuff[i] != GAMEDAT_HEAD[i]) { LowGameDat = true; break; }
	}
	//신버전에는 이런 헤더가 달려있어 판별이 가능하다

	if (LowGameDat) { 
		Low_GameDat_Xor (GameDatBuff, GameDatSize); 
		code = KOR_CODE;
	}
	else { 
		if ((GameDatBuff[0x22] == 0) || (GameDatBuff[0x1F] == 1)) { code = JAP_CODE; }
		else { code = KOR_CODE; }
		//*(type 3일 때만)언어코드 판별
	}

	Start_Offset = 0x20;
	for (unsigned int i = 0x20;i < GameDatSize;i++) {
		tp_strlen = *(unsigned int*)(GameDatBuff + i);
		if (tp_strlen < (GameDatSize - i)) {
			unsigned int j; buffpnt = &GameDatBuff[i + sizeof(int)];
			for (j = 0;j < tp_strlen;j++) { if (buffpnt[j] == 0) { break; } }
			if (j == (tp_strlen-1)) { Start_Offset = i; break; }
		}
		if (Start_Offset != 0x20) { break; }
	}
	//*그냥 시작 위치는 0x20부터 점검을 시작해서 텍스트 정보가 있는 곳을 지정하는 게 훨씬 나은 것 같다

	New_GameDatSize = Start_Offset;
	buffpnt = &GameDatBuff[Start_Offset];

	for (unsigned int i = 0;i < 9;i++) {

		tp_strlen = *(unsigned int*)buffpnt; buffpnt += sizeof(int);
		tmp = (char*)buffpnt; buffpnt += tp_strlen;
		if (tp_strlen != (strlen(tmp)+1)) {
			ttt.Format (_T("Game.dat 파일이 깨졌습니다.(%d)"), i);
			AfxMessageBox (ttt); free (GameDatBuff); return;
		}

		if ((i == 0) || (i == 3) || (i == 4) || (i == 5) || (i == 6)) {
			if (code == KOR_CODE) { tmpuni = K2U(tmp); }
			else if (code == JAP_CODE) { tmpuni = J2U(tmp); }
			if (i == 0) { Game_Title = tmpuni; }
			else if (i == 3) { Font1 = tmpuni; }
			else if (i == 4) { Font2 = tmpuni; }
			else if (i == 5) { Font3 = tmpuni; }
			else if (i == 6) { Font4 = tmpuni; }
			free (tmpuni);
		}
		else {
			if (i == 1) { Zeros = (char*)tmp; }
			else if (i == 2) { Spec = (char*)tmp; }
			else if (i == 7) { imgstr = (char*)tmp; }
			else if (i == 8) { version = (char*)tmp; }
		}
		//*각 변수에 값 할당(ansi 문자열만 필요하면 그냥 할당만 하고 넘어간다)
	}
	check_size = *(unsigned int*)buffpnt; buffpnt += sizeof(int);
	if (check_size != (GameDatSize - 1)) {
		AfxMessageBox (_T("Game.dat 파일이 깨졌습니다.(10)")); 
		free (GameDatBuff); return;
	}
	//*마지막 체크용 사이즈도 확인한다

	GameDat_Edit_Dialog dlg;
	dlg.Game_Title = Game_Title;
	dlg.Font1 = Font1; dlg.Font2 = Font2; dlg.Font3 = Font3; dlg.Font4 = Font4;
	if (LowGameDat) { dlg.type = DXA_LOW; } else { dlg.type = DXA_330; }
	//*사전정보 얻기/기입

	if (dlg.DoModal() == IDOK) {
	
		if (!LowGameDat) {
			if (dlg.code == KOR_CODE) { GameDatBuff[0x22] = 1; GameDatBuff[0x1F] = 2; }
			else if (dlg.code == JAP_CODE) { GameDatBuff[0x22] = 0; GameDatBuff[0x1F] = 1; }
			//*Type 3
			//최신 버전은 Game.dat에서 0x22번째 바이트가 언어코드를 담당한다.
			//0 : 일본어, 1 : 한국어. 여기까지만 알면 될듯.
			//또한 0x1F를 0x1 -> 0x2로 바꿔야 하는 것 같다.
		}
		
		Game_Title = dlg.Game_Title;
		Font1 = dlg.Font1; Font2 = dlg.Font2; Font3 = dlg.Font3; Font4 = dlg.Font4;
		//*다이얼로그에서 값 얻기

		for (unsigned int i = 0;i < 9;i++) {
			if ((i == 0) || (i == 3) || (i == 4) || (i == 5) || (i == 6)) {
				if (i == 0) { tmpuni = Game_Title.GetBuffer(); }
				else if (i == 3) { tmpuni = Font1.GetBuffer(); }
				else if (i == 4) { tmpuni = Font2.GetBuffer(); }
				else if (i == 5) { tmpuni = Font3.GetBuffer(); }
				else if (i == 6) { tmpuni = Font4.GetBuffer(); }
				if (dlg.code == KOR_CODE) { tmp = U2K(tmpuni); }
				else if (dlg.code == JAP_CODE) { tmp = U2J(tmpuni); }
				New_GameDatSize += (sizeof(int) + ((unsigned int)strlen(tmp) + 1));
				free(tmp);
			}
			else {
				if (i == 1) { tmp = Zeros; }
				else if (i == 2) { tmp = Spec; }
				else if (i == 7) { tmp = imgstr; }
				else if (i == 8) { tmp = version; }
				New_GameDatSize += (sizeof(int) + ((unsigned int)strlen(tmp) + 1));
			}
		}
		New_GameDatSize += (sizeof(int));
		New_GameDatSize += (GameDatSize - (unsigned int)(buffpnt - GameDatBuff));

		unsigned char *t_pnt = buffpnt;
		t_pnt += sizeof(int);
		unsigned int word_count = *(unsigned int*)t_pnt; t_pnt += sizeof(int);
		t_pnt += sizeof(short) * word_count;
		(*(unsigned int*)t_pnt) += New_GameDatSize;
		(*(unsigned int*)t_pnt) -= GameDatSize;
		t_pnt += sizeof(int);
		(*(unsigned int*)t_pnt) += New_GameDatSize;
		(*(unsigned int*)t_pnt) -= GameDatSize;
		t_pnt += sizeof(int);
		//*오차 조정

		New_GameDatBuff = (unsigned char*)malloc (sizeof(char) * New_GameDatSize);
		memset (New_GameDatBuff, 0, (sizeof(char) * New_GameDatSize));
		newbuffpnt = New_GameDatBuff;
		//*새 버퍼 길이 계산 및 할당
		//*위치값도 변경해야 한다

		memcpy (newbuffpnt, GameDatBuff, Start_Offset); newbuffpnt += Start_Offset;
		for (unsigned int i = 0;i < 9;i++) {
			if ((i == 0) || (i == 3) || (i == 4) || (i == 5) || (i == 6)) {
				if (i == 0) { tmpuni = Game_Title.GetBuffer(); }
				else if (i == 3) { tmpuni = Font1.GetBuffer(); }
				else if (i == 4) { tmpuni = Font2.GetBuffer(); }
				else if (i == 5) { tmpuni = Font3.GetBuffer(); }
				else if (i == 6) { tmpuni = Font4.GetBuffer(); }
				if (dlg.code == KOR_CODE) { tmp = U2K(tmpuni); }
				else if (dlg.code == JAP_CODE) { tmp = U2J(tmpuni); }
				*(unsigned int*)newbuffpnt = ((unsigned int)strlen(tmp) + 1); newbuffpnt += sizeof(int);
				memcpy (newbuffpnt, tmp, strlen(tmp)); newbuffpnt += (strlen(tmp) + 1);
				free(tmp);
			}
			else {
				if (i == 1) { tmp = Zeros; }
				else if (i == 2) { tmp = Spec; }
				else if (i == 7) { tmp = imgstr; }
				else if (i == 8) { tmp = version; }

				if (i == 8)  {
					*(unsigned int*)newbuffpnt = ((unsigned int)strlen(tmp) + 1); 
					newbuffpnt += sizeof(int);
					memcpy (newbuffpnt, tmp, strlen(tmp)); 
					newbuffpnt += (strlen(tmp) + 1);
				}
				else {
					*(unsigned int*)newbuffpnt = ((unsigned int)strlen(tmp) + 1); newbuffpnt += sizeof(int);
					memcpy (newbuffpnt, tmp, strlen(tmp)); newbuffpnt += (strlen(tmp) + 1);
				}
			}
		}
		*(unsigned int*)newbuffpnt = (New_GameDatSize - 1); 
		newbuffpnt += sizeof(int);
		memcpy (newbuffpnt, buffpnt, (GameDatSize - (buffpnt - GameDatBuff)));
		if (LowGameDat) { Low_GameDat_Xor (New_GameDatBuff, New_GameDatSize); }
		//***Game.dat 편집(새 버퍼로 이동), 낮은 버전은 다시 xor해준다

		_wremove (L"Data/BasicData/Game.dat");
		hGameDat = CreateFileW (L"Data/BasicData/Game.dat", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile (hGameDat, New_GameDatBuff, New_GameDatSize, NULL, NULL);
		CloseHandle (hGameDat); free (New_GameDatBuff);
		//*편집 후 파일에 새로 기록

		AfxMessageBox (_T("Game.dat 파일을 저장했습니다."));

	}

	free (GameDatBuff);
	//*버퍼 해제

}