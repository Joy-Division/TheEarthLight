/*
 * Sound Driver for PS2 IOP
 * Memory Stream System
 *
 * ver."ZONE OF THE ENDERS"
 */
#include <sys/types.h>
#include <kernel.h>
#include <libsd.h>

#include "sd_incl.h"
#include "sd_ext.h"

/*---------------------------------------------------------------------------*/

void init_mem_str_w( void )
{
	int i;
	
	for( i = 0 ; i < 12 ; i++ ){
		mem_str_w[i].unk00 = 0;
	}
}

/*---------------------------------------------------------------------------*/

int MemSpuTransWithNoLoop( u_int a0 )
{
	int temp2 = 0;
	u_int temp3;
	struct unkstr24 *temp4;
	u_short temp5;
	
	a0 -= 32;
	temp4 = &mem_str_w[a0];
	
/* ///////////////////////////////////////////////////////////////////////// */
	switch( (temp4->unk00 & 0xF)-2 ){
	case 0:
		temp4->unk04 = temp4->unk08 = voice_tbl[temp4->unk0C+1].addr - voice_tbl[temp4->unk0C].addr;
		temp4->unk10 = mem_str_buf+voice_tbl[temp4->unk0C].addr;
		temp4->unk10[17] = 0;
		temp4->unk10[temp4->unk04-0x1F] = 1;
		
		sceSdSetAddr( (SD_CORE_1|((a0+8)<<1)|SD_VA_LSAX), a0*0x1000+0x9020 );
		sceSdVoiceTrans( 0, 0, temp4->unk10, (u_char *)(a0*0x1000+0x9020), 0x800 );
		
		temp4->unk10 += 0x0800;
		temp4->unk08 -= 0x0800;
		temp4->unk00++;
		temp2 = 1;
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 1:
		if( !temp4->unk08 || temp4->unk08 & 0x80000000 ){
			temp4->unk00++;
		} else {
			if( temp4->unk08 > 0x0800 && temp4->unk10[0x07F1] != 1 ){
				temp4->unk10[0x07F1] |= 3;
			}
			sceSdVoiceTrans( 0, 0, temp4->unk10, (u_char *)(a0*0x1000+0x9820), 0x0800 );
			temp4->unk10 += 0x0800;
			temp4->unk08 -= 0x0800;
			temp4->unk00++;
			temp2 = 1;
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 2:
		sceSdSetAddr( (SD_CORE_1|((a0+8)<<1)|SD_VA_SSA), a0*0x1000+0x9020 );
		keyon();
		temp4->unk14 = 0x0800;
		temp4->unk00++;
		if( !temp4->unk08 || temp4->unk08 & 0x80000000 ){
			temp4->unk00++;
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 3:
		// FIXME: Adjust these to use the provided macros from the SDK
		temp5 = sceSdGetParam( (a0+8)*2 | 0x0501 );
		if( !temp5 ){
			temp4->unk00++;
		}
		// FIXME: Adjust these to use the provided macros from the SDK
		temp3 = sceSdGetAddr( (a0+8)*2 | 0x2241 );
		temp3 -= 0x9020 + (a0*0x1000);
		if( temp3 >= 0x1000 ){
			break;
		}
		if( temp4->unk14 == (temp3 & 0x0800) ){
			temp2 = 1;
			if( temp3 >= 0x0800 ){
				sceSdVoiceTrans( 0, 0, temp4->unk10, (u_char *)(a0*0x1000+0x9020), 0x0800 );
				temp4->unk14 = 0;
			} else {
				if( temp4->unk08 > 0x0800 && temp4->unk10[0x07F1] != 1 ){
					temp4->unk10[0x07F1] |= 3;
				}
				sceSdVoiceTrans( 0, 0, temp4->unk10, (u_char *)(a0*0x1000+0x9820), 0x0800 );
				temp4->unk14 = 0x0800;
			}
			temp4->unk10 += 0x0800;
			if( temp4->unk08 > 0x0800 ){
				temp4->unk08 -= 0x0800;
			} else {
				temp4->unk00++;
			}
		} else {
			// EMPTY BLOCK
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 4:
		temp4->unk00++;
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 5:
		break;
	}
	return temp2;
}

/*---------------------------------------------------------------------------*
 * END OF FILE
 *---------------------------------------------------------------------------*/
/* -*- indent-tabs-mode: t; tab-width: 4; mode: c; -*- */
/* vim: set noet ts=4 sw=4 ft=c ff=unix fenc=euc-jp : */
