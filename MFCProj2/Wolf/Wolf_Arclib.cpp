#include "stdafx.h"

#include "Wolf_Arclib.h"

void Decode (unsigned char *buff, unsigned int buffsize)
{
	u32 srcsize, destsize, code, indexsize, keycode, conbo, index ;
	u8 *srcp, *destp, *dp, *sp ;

	//맨 앞자리 32비트를 확인해서 그 값이 덩어리 크기와 같을 때만 압축을 풀도록 한다
	srcp  = (u8 *)buff;
	destsize = *((u32 *)&srcp[0]) ;
	srcsize = *((u32 *)&srcp[4]) - 9 ;

	if (destsize != buffsize) { return; }

	destp = (u8 *)malloc(sizeof(char)*destsize);

	keycode = srcp[8] ;
	
	sp  = srcp + 9 ;
	dp  = destp ;
	while( srcsize )
	{
		if( sp[0] != keycode )
		{
			*dp = *sp ;
			dp      ++ ;
			sp      ++ ;
			srcsize -- ;
			continue ;
		}
	
		if( sp[1] == keycode )
		{
			*dp = (u8)keycode ;
			dp      ++ ;
			sp      += 2 ;
			srcsize -= 2 ;
			
			continue ;
		}

		code = sp[1] ;

		if( code > keycode ) code -- ;

		sp      += 2 ;
		srcsize -= 2 ;

		conbo = code >> 3 ;
		if( code & ( 0x1 << 2 ) )
		{
			conbo |= *sp << 5 ;
			sp      ++ ;
			srcsize -- ;
		}
		conbo += MIN_COMPRESS ;

		indexsize = code & 0x3 ;
		switch( indexsize )
		{
		case 0 :
			index = *sp ;
			sp      ++ ;
			srcsize -- ;
			break ;
			
		case 1 :
			index = *((u16 *)sp) ;
			sp      += 2 ;
			srcsize -= 2 ;
			break ;
			
		case 2 :
			index = *((u16 *)sp) | ( sp[2] << 16 ) ;
			sp      += 3 ;
			srcsize -= 3 ;
			break ;
		}
		index ++ ;

		if( index < conbo )
		{
			u32 num;

			num  = index ;
			while( conbo > num )
			{
				memcpy( dp, dp - num, num ) ;
				dp    += num ;
				conbo -= num ;
				num   += num ;
			}
			if( conbo != 0 )
			{
				memcpy( dp, dp - num, conbo ) ;
				dp += conbo ;
			}
		}
		else
		{
			memcpy( dp, dp - index, conbo ) ;
			dp += conbo ;
		}
	}

	memcpy (buff, destp, destsize);
	free (destp);
	return;
}

void Encode (unsigned char *buff, unsigned int SrcSize)
{
	s32 dstsize ;
	s32    bonus,    conbo,    conbosize,    address,    addresssize ;
	s32 maxbonus, maxconbo, maxconbosize, maxaddress, maxaddresssize ;
	u8 keycode, *srcp, *destp, *dp, *sp, *sp2, *sp1 ;
	u32 srcaddress, nextprintaddress, code ;
	s32 j ;
	u32 i, m ;
	u32 maxlistnum, maxlistnummask, listaddp ;
	u32 sublistnum, sublistmaxnum ;
	LZ_LIST *listbuf, *listtemp, *list, *newlist ;
	u8 *listfirsttable, *usesublistflagtable, *sublistbuf ;
	
	{
			 if( SrcSize < 100 * 1024 )			sublistmaxnum = 1 ;
		else if( SrcSize < 3 * 1024 * 1024 )	sublistmaxnum = MAX_SUBLISTNUM / 3 ;
		else									sublistmaxnum = MAX_SUBLISTNUM ;
	}

	{
		maxlistnum = MAX_ADDRESSLISTNUM ;
		if( maxlistnum > SrcSize )
		{
			while( ( maxlistnum >> 1 ) > 0x100 && ( maxlistnum >> 1 ) > SrcSize )
				maxlistnum >>= 1 ;
		}
		maxlistnummask = maxlistnum - 1 ;
	}

	usesublistflagtable   = (u8 *)malloc(
		sizeof( LZ_LIST* )     * 65536 +	
		sizeof( LZ_LIST ) * maxlistnum +
		sizeof( u8 )      * 65536 +	
		sizeof( LZ_LIST* )     * 256 * sublistmaxnum ) ;	
		
	listfirsttable =     usesublistflagtable + sizeof(  u8 ) * 65536 ;
	sublistbuf     =          listfirsttable + sizeof( LZ_LIST* ) * 65536 ;
	listbuf        = (LZ_LIST *)( sublistbuf + sizeof( LZ_LIST* ) * 256 * sublistmaxnum ) ;
	
	memset( usesublistflagtable, 0, sizeof(  u8 ) * 65536               ) ;
	memset(          sublistbuf, 0, sizeof( LZ_LIST* ) * 256 * sublistmaxnum ) ;
	memset(      listfirsttable, 0, sizeof( LZ_LIST* ) * 65536               ) ;
	list = listbuf ;
	for( i = maxlistnum / 8 ; i ; i --, list += 8 )
	{
		list[0].address =
		list[1].address =
		list[2].address =
		list[3].address =
		list[4].address =
		list[5].address =
		list[6].address =
		list[7].address = 0xffffffff ;
	}

	srcp  = (u8 *)buff;
	destp = (u8 *)malloc(sizeof(char)*SrcSize);

	{
		u32 qnum, table[256], mincode ;

		for( i = 0 ; i < 256 ; i ++ )
			table[i] = 0 ;
		
		sp   = srcp ;
		qnum = SrcSize / 8 ;
		i    = qnum * 8 ;
		for( ; qnum ; qnum --, sp += 8 )
		{
			table[sp[0]] ++ ;
			table[sp[1]] ++ ;
			table[sp[2]] ++ ;
			table[sp[3]] ++ ;
			table[sp[4]] ++ ;
			table[sp[5]] ++ ;
			table[sp[6]] ++ ;
			table[sp[7]] ++ ;
		}
		for( ; i < SrcSize ; i ++, sp ++ )
			table[*sp] ++ ;
			
		keycode = 0 ;
		mincode = table[0] ;
		for( i = 1 ; i < 256 ; i ++ )
		{
			if( mincode < table[i] ) continue ;
			mincode = table[i] ;
			keycode = (u8)i ;
		}
	}

	((u32 *)destp)[0] = SrcSize ;

	destp[8] = keycode ;

	dp               = destp + 9 ;
	sp               = srcp ;
	srcaddress       = 0 ;
	dstsize          = 0 ;
	listaddp         = 0 ;
	sublistnum       = 0 ;
	nextprintaddress = 1024 * 100 ;
	while( srcaddress < SrcSize )
	{
		if( srcaddress + MIN_COMPRESS >= SrcSize ) goto NOENCODE ;

		code = *((u16 *)sp) ;
		list = (LZ_LIST *)( listfirsttable + code * sizeof( LZ_LIST* ) ) ;
		if( usesublistflagtable[code] == 1 )
		{
#ifdef WIN64
			list = (LZ_LIST *)( (u32*)list->next + sp[2]*2 ) ;	//64bit
#else
			list = (LZ_LIST *)( (u32*)list->next + sp[2] ) ;	//32bit
#endif
		}
		else
		{
			if( sublistnum < sublistmaxnum )
			{
				list->next = (LZ_LIST *)( sublistbuf + sizeof( LZ_LIST* ) * 256 * sublistnum ) ;
#ifdef WIN64
				list       = (LZ_LIST *)( (u32*)list->next + sp[2]*2 ) ;	//64bit
#else
				list       = (LZ_LIST *)( (u32*)list->next + sp[2] ) ;		//32bit
#endif
			
				usesublistflagtable[code] = 1 ;
				sublistnum ++ ;
			}
		}

		maxconbo   = -1 ;
		maxaddress = -1 ;
		maxbonus   = -1 ;
		for( m = 0, listtemp = list->next ; m < MAX_SEARCHLISTNUM && listtemp != NULL ; listtemp = listtemp->next, m ++ )
		{
			address = srcaddress - listtemp->address ;
			if( address >= MAX_POSITION )
			{
				if( listtemp->prev ) listtemp->prev->next = listtemp->next ;
				if( listtemp->next ) listtemp->next->prev = listtemp->prev ;
				listtemp->address = 0xffffffff ;
				continue ;
			}
			
			sp2 = &sp[-address] ;
			sp1 = sp ;
			if( srcaddress + MAX_COPYSIZE < SrcSize )
			{
				conbo = MAX_COPYSIZE / 4 ;
				while( conbo && *((u32 *)sp2) == *((u32 *)sp1) )
				{
					sp2 += 4 ;
					sp1 += 4 ;
					conbo -- ;
				}
				conbo = MAX_COPYSIZE - ( MAX_COPYSIZE / 4 - conbo ) * 4 ;

				while( conbo && *sp2 == *sp1 )
				{
					sp2 ++ ;
					sp1 ++ ;
					conbo -- ;
				}
				conbo = MAX_COPYSIZE - conbo ;
			}
			else
			{
				for( conbo = 0 ;
						conbo < MAX_COPYSIZE &&
						conbo + srcaddress < SrcSize &&
						sp[conbo - address] == sp[conbo] ;
							conbo ++ ){}
			}

			if( conbo >= 4 )
			{
				conbosize   = ( conbo - MIN_COMPRESS ) < 0x20 ? 0 : 1 ;
				addresssize = address < 0x100 ? 0 : ( address < 0x10000 ? 1 : 2 ) ;
				bonus       = conbo - ( 3 + conbosize + addresssize ) ;

				if( bonus > maxbonus )
				{
					maxconbo       = conbo ;
					maxaddress     = address ;
					maxaddresssize = addresssize ;
					maxconbosize   = conbosize ;
					maxbonus       = bonus ;
				}
			}
		}

		newlist = &listbuf[listaddp] ;
		if( newlist->address != 0xffffffff )
		{
			if( newlist->prev ) newlist->prev->next = newlist->next ;
			if( newlist->next ) newlist->next->prev = newlist->prev ;
			newlist->address = 0xffffffff ;
		}
		newlist->address = srcaddress ;
		newlist->prev    = list ;
		newlist->next    = list->next ;
		if( list->next != NULL ) list->next->prev = newlist ;
		list->next       = newlist ;
		listaddp         = ( listaddp + 1 ) & maxlistnummask ;

		if( maxconbo == -1 )
		{
NOENCODE:
			if( *sp == keycode )
			{
				if( destp != NULL )
				{
					dp[0]  =
					dp[1]  = keycode ;
					dp += 2 ;
				}
				dstsize += 2 ;
			}
			else
			{
				if( destp != NULL )
				{
					*dp = *sp ;
					dp ++ ;
				}
				dstsize ++ ;
			}
			sp ++ ;
			srcaddress ++ ;
		}
		else
		{
			if( destp != NULL )
			{
				*dp++ = keycode ;

				maxconbo -= MIN_COMPRESS ;

				*dp = (u8)( ( ( maxconbo & 0x1f ) << 3 ) | ( maxconbosize << 2 ) | maxaddresssize ) ;

				if( *dp >= keycode ) dp[0] += 1 ;
				dp ++ ;

				if( maxconbosize == 1 )
					*dp++ = (u8)( ( maxconbo >> 5 ) & 0xff ) ;

				maxconbo += MIN_COMPRESS ;

				maxaddress -- ;

				*dp++ = (u8)( maxaddress ) ;
				if( maxaddresssize > 0 )
				{
					*dp++ = (u8)( maxaddress >> 8 ) ;
					if( maxaddresssize == 2 )
						*dp++ = (u8)( maxaddress >> 16 ) ;
				}
			}
			
			dstsize += 3 + maxaddresssize + maxconbosize ;
			
			if( srcaddress + maxconbo < SrcSize )
			{
				sp2 = &sp[1] ;
				for( j = 1 ; j < maxconbo && (u32)&sp2[2] - (u32)srcp < SrcSize ; j ++, sp2 ++ )
				{
					code = *((u16 *)sp2) ;
					list = (LZ_LIST *)( listfirsttable + code * sizeof( LZ_LIST* ) ) ;
					if( usesublistflagtable[code] == 1 )
					{
#ifdef WIN64
						list = (LZ_LIST *)( (u32*)list->next + sp2[2]*2 ) ;	//64bit
#else
						list = (LZ_LIST *)( (u32*)list->next + sp2[2] ) ;		//32bit
#endif
					}
					else
					{
						if( sublistnum < sublistmaxnum )
						{
							list->next = (LZ_LIST *)( sublistbuf + sizeof( LZ_LIST* ) * 256 * sublistnum );
#ifdef WIN64
							list       = (LZ_LIST *)( (u32*)list->next + sp2[2]*2 ) ;	//64bit
#else
							list       = (LZ_LIST *)( (u32*)list->next + sp2[2] ) ;	//32bit
#endif
						
							usesublistflagtable[code] = 1 ;
							sublistnum ++ ;
						}
					}

					newlist = &listbuf[listaddp] ;
					if( newlist->address != 0xffffffff )
					{
						if( newlist->prev ) newlist->prev->next = newlist->next ;
						if( newlist->next ) newlist->next->prev = newlist->prev ;
						newlist->address = 0xffffffff ;
					}
					newlist->address = srcaddress + j ;
					newlist->prev = list ;
					newlist->next = list->next ;
					if( list->next != NULL ) list->next->prev = newlist ;
					list->next = newlist ;
					listaddp = ( listaddp + 1 ) & maxlistnummask ;
				}
			}
			
			sp         += maxconbo ;
			srcaddress += maxconbo ;
		}

		if( nextprintaddress < srcaddress )
		{
			nextprintaddress = srcaddress + 100 * 1024 ;
		}
	}

	*((u32 *)&destp[4]) = dstsize + 9 ;

	free( usesublistflagtable ) ;

	memset (buff, 0, SrcSize);
	memcpy (buff, destp, dstsize+9);
	free (destp);
	return;
}