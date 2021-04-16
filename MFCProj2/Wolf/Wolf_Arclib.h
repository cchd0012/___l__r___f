#include <stdio.h>
#include <Windows.h>

#define u32 unsigned int
#define s32 int
#define u16 unsigned short
#define u8 unsigned char

#define MIN_COMPRESS		(4)
#define MAX_SEARCHLISTNUM	(64)
#define MAX_SUBLISTNUM		(65536)
#define MAX_COPYSIZE 		(0x1fff + MIN_COMPRESS)
#define MAX_ADDRESSLISTNUM	(1024 * 1024 * 1)
#define MAX_POSITION		(1 << 24)

typedef struct LZ_LIST
{
	LZ_LIST *next, *prev ;
	u32 address ;
} LZ_LIST ;

void Decode (unsigned char *buff, unsigned int buffsize);
//압축풀이 함수

void Encode (unsigned char *buff, unsigned int SrcSize);
//압축하는 함수