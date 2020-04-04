/*
 * Sound Driver for PS2 IOP
 * File Loader System
 */
/* ver."ZONE OF THE ENDERS"
 */
#include <sys/types.h>
#include <stdio.h>
#include <kernel.h>
#include "sd_incl.h"
#include "sd_ext.h"

/*---------------------------------------------------------------------------*/

static unsigned int pakcd_pos;

/* unreferenced TGS2000 variables */
u_int gsize = 0;

u_int pak_cd_read_fg = 0;
u_int save_wvx1 = -1, save_wvx2 = -1;
u_int save_efx = -1;
u_int save_mdx = -1;

/*---------------------------------------------------------------------------*/

void LoadPakFile( void )
{
	struct unkstr08 *temp = (struct unkstr08 *)pak_header;

	switch( pak_load_status-1 ){
	case 0:
		if( pak_read_fg ){
			if( pak_fp ){
				PcmClose( pak_fp );
				pak_fp = 0;
		}
			pak_read_fg = 0;
		}
		pak_fp = PcmOpen( pak_load_code, 5 );
		if( pak_fp < 0 ){

			// Wait for 4 V-blanks
			WaitVblankStart(); WaitVblankEnd(); // 1st interval
			WaitVblankStart(); WaitVblankEnd(); // 2nd interval
			WaitVblankStart(); WaitVblankEnd(); // 3rd interval
			WaitVblankStart(); WaitVblankEnd(); // 4th interval

			pak_load_code = 0;
			pak_fp = 0;
			pak_load_status = 0;
			break;
		} else {
			//
			// EMPTY BLOCK
			//
		}
		pak_read_fg = 1;
		PcmRead( pak_fp, pak_header, 0x0800 );
		if( temp[0].unk00 ){
			if( temp[0].unk04 != save_wvx1 && temp[0].unk04 != save_wvx2 ){
				d1E0E4 = (temp[1].unk00 - temp[0].unk00) * 0x0800; // guessed varname
				save_wvx1 = temp[0].unk04;
				wave_load_code = 0xFEFFFFFE;
				wave_load_status = 1;
				pak_load_status = 2;
			} else {
				PcmLseek( pak_fp, (temp[1].unk00 - temp[0].unk00) * 0x0800, 1 );
				pak_load_status = 3;
			}
		} else {
			pak_load_status = 9;
		}
		break;

	case 1:
		if( !wave_load_status ){
			pak_load_status = 3;
		}
		break;

	case 2:
		if( temp[1].unk00 ){
			if( temp[1].unk04 != save_wvx1 && temp[1].unk04 != save_wvx2 ){
				d1E0E4 = (temp[2].unk00 - temp[1].unk00) * 0x0800; // guessed varname
				save_wvx2 = temp[1].unk04;
				wave_load_code = 0xFEFFFFFF;
				wave_load_status = 1;
				pak_load_status = 4;
			} else {
				PcmLseek( pak_fp, (temp[2].unk00 - temp[1].unk00) * 0x0800, 1 );
				pak_load_status = 5;
			}
		} else {
			pak_load_status = 9;
		}
		break;

	case 3:
		if( !wave_load_status ){
			pak_load_status = 5;
		}
		break;

	case 4:
		if( temp[2].unk00 ){
			if( temp[2].unk04 != save_efx ){
				save_efx = temp[2].unk04;
				se_load_code = 0x02FFFFFF;
				PcmRead( pak_fp, se_exp_table, (temp[3].unk00 - temp[2].unk00) * 0x0800 );
				se_load_code = 0;
			} else {
				PcmLseek( pak_fp, (temp[3].unk00 - temp[2].unk00) * 0x0800, 1 );
			}
		} else {
			//
			// EMPTY BLOCK
			//
		}
		pak_load_status = 6;
		break;

	case 5:
		if( temp[3].unk00 ){
			if( temp[3].unk04 != save_mdx ){
				save_mdx = temp[3].unk04;
				d1E0E8 = 0x010000FF; // guessed varname
				pak_load_status = 7;
			} else if(1) {
				pak_load_status = 9;
				break;
			}
		} else {
			pak_load_status = 9;
			break;
		}
		break; // NOTICE

	case 6:
		if( d1E0E8 ){ // guessed varname
			break;
		} else {
			pak_load_status = 8;
		}
		break;

	case 7:
		if( sng_load_code ){
			break;
		} else {
			pak_load_status = 9;
		}
		break;

	case 8:
		pak_read_fg = 0;
		PcmClose( pak_fp );
		pak_fp = 0;
		pak_load_status = 0;
		break;
	}
}

/*---------------------------------------------------------------------------*/

#define SD_PATH_SD1_DUMMY  0
#define SD_PATH_SD1_VOX1   1
#define SD_PATH_SD1_WVX1   2
#define SD_PATH_SD1_MDX1   3
#define SD_PATH_SD1_EFX1   4
#define SD_PATH_SD1_SDX1   5

char *sd_path_sd1[6] = {
	"\\DUMMY\\",
	"host0:./sound/vox1/",
	"host0:./sound/wvx1/",
	"host0:./sound/mdx1/",
	"host0:./sound/efx1/",
	"host0:./sound/sdx1/"
};

/*---------------------------------------------------------------------------*/

int LoadSeFile( void )
{
	if( se_fp ){
		PcmClose( se_fp );
		se_fp = 0;
	}

	se_fp = PcmOpen( se_load_code, 4 );

	if( se_fp < 0 ){
		se_load_code = 0;
		se_fp = 0;
		return -1;
	}

	PcmRead( se_fp, se_exp_table, 0x4800 );
	PcmClose( se_fp );

	se_load_code = 0;
	se_fp = 0;
	return 0;
}

/*---------------------------------------------------------------------------*/

int LoadSngData()
{
	int temp;

	sng_fp = PcmOpen( sng_load_code, 3 );

	if( sng_fp < 0 ){
		sng_load_code = 0;
		sng_fp = 0;
		return -1;
	}

	temp = PcmRead( sng_fp, sng_data, 0x20000 );

	PcmClose( sng_fp );

	sng_load_code = 0;
	sng_fp = 0;
	return 0;
}

/*---------------------------------------------------------------------------*/

void set_voice_tbl( u_int *a0, u_int a1, u_int a2 )
{
	u_int temp;
	int temp2 = 0, temp3, temp4;

	temp4 = -1;

	for( temp = a2/16; temp < a1/16; temp++ ){
		temp3 = a0[temp*4];
		if( !temp2 ) {
			if( temp3 < temp4 ){
				temp2 = 1;
				break;
			} else {
				temp4 = temp3;
			}
		}
	}

	memcpy( (u_char *)voice_tbl + a2, a0, temp * 16 );

	if( temp2 ){
		memcpy( drum_tbl, temp*4 + a0, a1 - temp*16 );
	}
}

/*---------------------------------------------------------------------------*/

int LoadWaveFile( void )
{
	u_int temp, temp2, temp3, temp4;

	if( wave_fp ){
		PcmClose( wave_fp );
		wave_fp = 0;
	}

	wave_fp = PcmOpen( wave_load_code, 2 );

	if( wave_fp < 0 ){
		wave_fp = 0;

		// Wait for 4 V-blanks
		WaitVblankStart(); WaitVblankEnd(); // 1st interval
		WaitVblankStart(); WaitVblankEnd(); // 2nd interval
		WaitVblankStart(); WaitVblankEnd(); // 3rd interval
		WaitVblankStart(); WaitVblankEnd(); // 4th interval

		wave_load_code = 0;
		return -1;
	}

	if( wave_load_code <= 0xFEFFFFFD ){
		temp4 = 0x00018000;
	} else if( d1E0E4 <= 0x00017FFF ){
		temp4 = d1E0E4;
	} else {
		temp4 = 0x00018000;
	}

	PcmRead( wave_fp, cdload_buf, temp4 );

	temp =  cdload_buf[0] << 24;
	temp |= cdload_buf[1] << 16;
	temp |= cdload_buf[2] << 8;
	temp |= cdload_buf[3];

	temp2 =  cdload_buf[4] << 24;
	temp2 |= cdload_buf[5] << 16;
	temp2 |= cdload_buf[6] << 8;
	temp2 |= cdload_buf[7];

	wave_load_ptr = cdload_buf+16;
	set_voice_tbl( (u_int *)wave_load_ptr, temp2, temp );
	wave_load_ptr += temp2;

	spu_load_offset =  wave_load_ptr[0] << 24;
	spu_load_offset |= wave_load_ptr[1] << 16;
	spu_load_offset |= wave_load_ptr[2] << 8;
	spu_load_offset |= wave_load_ptr[3];

	wave_unload_size =  wave_load_ptr[4] << 24;
	wave_unload_size |= wave_load_ptr[5] << 16;
	wave_unload_size |= wave_load_ptr[6] << 8;
	wave_unload_size |= wave_load_ptr[7];

	wave_load_ptr += 16;

	if( (0x00017FE0 - temp2) < wave_unload_size ){
		wave_load_size = 0x00017FE0 - temp2;
	} else {
		wave_load_size = wave_unload_size;
	}

	wave_unload_size = wave_unload_size - wave_load_size;
	wave_save_code = wave_load_code;

	if( temp >= 0x1000 ){
		memcpy( mem_str_buf, wave_load_ptr, wave_load_size );
		temp3 = wave_load_size;
		while( wave_unload_size ){
			if( wave_unload_size > 0x00018000 ){
				PcmRead( wave_fp, cdload_buf, 0x18000 );
				wave_load_size = 0x00018000;
				wave_unload_size -= 0x00018000;
			} else if( wave_unload_size ){
				PcmRead( wave_fp, cdload_buf, wave_unload_size );
				wave_load_size = wave_unload_size;
				wave_unload_size = 0;
			}
			else {
			}
			memcpy( mem_str_buf+temp3, cdload_buf, wave_load_size );
			temp3 += wave_load_size;
			if( temp3 > 0x000A0000 ){
				break;
			}
		}
		PcmClose( wave_fp );
		wave_fp = 0;
		wave_load_code = 0;
		return 1;
		return; // SYNTAX NOTICE
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

void WaveCdLoad( void )
{
	// Wait for 4 V-blanks
	WaitVblankEnd(); // 1st interval (end-only)
	WaitVblankStart(); WaitVblankEnd(); // 2nd interval
	WaitVblankStart(); WaitVblankEnd(); // 3rd interval
	WaitVblankStart(); WaitVblankEnd(); // 4th interval

	if( wave_unload_size > 0x00018000 ){
		PcmRead( wave_fp, cdload_buf, 0x00018000 );
		wave_load_ptr = cdload_buf;
		wave_load_size = 0x00018000;
		wave_unload_size -= 0x00018000;
		wave_load_status = 2;
	} else if( wave_unload_size ){
		PcmRead( wave_fp, cdload_buf, wave_unload_size );
		wave_load_ptr = cdload_buf;
		wave_load_size = wave_unload_size;
		wave_unload_size = 0;
		wave_load_status = 2;
	} else {
		wave_load_status = 0;
		PcmClose( wave_fp );
		wave_fp = 0;
		wave_load_code = 0;
	}
}

/*---------------------------------------------------------------------------*/

void WaveSpuTrans( void )
{
	int result;

	if( wave_load_status == 2 ){
		result = sceSdVoiceTrans(
			0,                                    /* transfer channel */
			SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,  /* transfer mode    */
			wave_load_ptr,                        /* IOP memory addr  */
			spu_wave_start_ptr + spu_load_offset, /* SPU memory addr  */
			wave_load_size );                     /* transfer size    */

		if( result < 0 ){
			//
			// EMPTY BLOCK
			//
		} else {
			spu_load_offset += wave_load_size;
		}
		wave_load_status += 1;
	}
}

/*---------------------------------------------------------------------------*/

/* ------------------------------------------------ */
/* Name Format  | Type  | Content                   */
/* -------------+-------+-------------------------- */
/* sg%06x.mdx   | SONG  | Sequence Data (BGM)       */
/* se%06x.efx   | SE    | Sequence Data (SE)        */
/* vc%06x.pcm   | VOICE | ADPCM Stream              */
/* wv%06x.wvx   | WAVE  | ADPCM Sample Bank         */
/* pk%06x.sdx   | PACK  | MDX+EFX+WVX Container     */
/* ------------------------------------------------ */

void code2name( u_int code, char *name )
{
	/* --- Song Sequence Data --- */
	if( code > 0x00FFFFFF && code <= 0x0100FFFF ){
		name[ 0] = 's';
		name[ 1] = 'g';
		name[ 2] = num2char( (code >> 20) & 0x0F );
		name[ 3] = num2char( (code >> 16) & 0x0F );
		name[ 4] = num2char( (code >> 12) & 0x0F );
		name[ 5] = num2char( (code >>  8) & 0x0F );
		name[ 6] = num2char( (code >>  4) & 0x0F );
		name[ 7] = num2char( (code      ) & 0x0F );
		name[ 8] = '.';
		name[ 9] = 'm';
		name[10] = 'd';
		name[11] = 'x';
		name[12] = 0;
	}
	/* --- Sound Effect Sequence Data --- */
	else if( code > 0x01FFFFFF && code <= 0x0200FFFF ){
		name[ 0] = 's';
		name[ 1] = 'e';
		name[ 2] = num2char( (code >> 20) & 0x0F );
		name[ 3] = num2char( (code >> 16) & 0x0F );
		name[ 4] = num2char( (code >> 12) & 0x0F );
		name[ 5] = num2char( (code >>  8) & 0x0F );
		name[ 6] = num2char( (code >>  4) & 0x0F );
		name[ 7] = num2char( (code      ) & 0x0F );
		name[ 8] = '.';
		name[ 9] = 'e';
		name[10] = 'f';
		name[11] = 'x';
		name[12] = 0;
	} else {
		/* --- ADPCM Stream --- */
		if( code > 0xEFFFFFFF && code <= 0xF0FFFFFF ){
			name[ 0] = 'v';
			name[ 1] = 'c';
			name[ 2] = num2char( (code >> 20) & 0x0F );
			name[ 3] = num2char( (code >> 16) & 0x0F );
			name[ 4] = num2char( (code >> 12) & 0x0F );
			name[ 5] = num2char( (code >>  8) & 0x0F );
			name[ 6] = num2char( (code >>  4) & 0x0F );
			name[ 7] = num2char( (code      ) & 0x0F );
			name[ 8] = '.';
			name[ 9] = 'p';
			name[10] = 'c';
			name[11] = 'm';
			name[12] = 0;
		}
		/* --- ADPCM Wave Bank --- */
		if( code > 0xFDFFFFFF && code <= 0xFE00FFFF ){
			name[ 0] = 'w';
			name[ 1] = 'v';
			name[ 2] = num2char( (code >> 20) & 0x0F );
			name[ 3] = num2char( (code >> 16) & 0x0F );
			name[ 4] = num2char( (code >> 12) & 0x0F );
			name[ 5] = num2char( (code >>  8) & 0x0F );
			name[ 6] = num2char( (code >>  4) & 0x0F );
			name[ 7] = num2char( (code      ) & 0x0F );
			name[ 8] = '.';
			name[ 9] = 'w';
			name[10] = 'v';
			name[11] = 'x';
			name[12] = 0;
		}
		/* --- MDX+EFX+WVX Container --- */
		if( code > 0xFE7FFFFF && code <= 0xFEFFFFFF ){
			name[ 0] = 'p';
			name[ 1] = 'k';
			name[ 2] = num2char( (code >> 20) & 0x07 ); // NOTICE
			name[ 3] = num2char( (code >> 16) & 0x0F );
			name[ 4] = num2char( (code >> 12) & 0x0F );
			name[ 5] = num2char( (code >>  8) & 0x0F );
			name[ 6] = num2char( (code >>  4) & 0x0F );
			name[ 7] = num2char( (code      ) & 0x0F );
			name[ 8] = '.';
			name[ 9] = 's';
			name[10] = 'd';
			name[11] = 'x';
			name[12] = 0;
		}
	}
}

/*---------------------------------------------------------------------------*/

char num2char( u_int num )
{
	num &= 0x0F;

	if( num < 10 ){
		num += 0x30; /* ASCII '0'～'9' */
	} else {
		num += 0x57; /* ASCII 'a'～'f' */
	}
	return num;
}

/*---------------------------------------------------------------------------*/

void str_cat( char *dst, char *src )
{
	int i;

	for( i = 0 ; i < 256 ; i++ ){
		if( !dst[i] ) break;
	}

	if( i < 256 ){
		strcpy( dst+i, src );
	}
}

/*---------------------------------------------------------------------------*/

int PcmOpen( u_int code, u_int path_idx )
{
	int status;
	char path[128], filename[16];

	if( pak_read_fg ){
		return pak_fp;
	}

	if( pak_cd_read_fg ){
		cdOpen();
		pakcd_pos = pak_load_code;
		return 1;
	}

	if( path_idx != SD_PATH_SD1_SDX1 || !path_name[0] ){
		strcpy( path, sd_path_sd1[path_idx] );
	} else {
		strcpy( path, path_name );
	}

	code2name( code, filename );
	str_cat( path, filename );

	if( !pak_read_fg ){
		//
		// EMPTY BLOCK
		//
	}

	status = pcOpen( path, 1 );

	if( status < 0 ){
		printf( "PcmOpen Error(%x)\n", status );
	}

	return status;
}

/*---------------------------------------------------------------------------*/

int PcmRead( int a0, void *a1, int a2 )
{
	int temp3;
	int temp;

	a2 = (a2 + 0x07FF) & 0xFFFFF800;

	if( pak_cd_read_fg ){
		cdRead( pakcd_pos, a1, a2 );
		//~ if( a2 < 0 );
		pakcd_pos += a2/2048;
		return a2;
	}

	temp3 = pcRead( a0, a1, a2 );

	if( temp3 <= 0 ){
		printf( "PcmRead Error(%x:size=%x)\n", temp3, a2 );
	}
	return temp3;
}

/*---------------------------------------------------------------------------*/

int PcmLseek( int a0, u_int a1, u_int a2 )
{
	int temp3;

	if( pak_cd_read_fg ){
		if( a2 == 1 ){
			pakcd_pos += a1 >> 11;
		}
		return a0;
	}

	temp3 = pcLseek( a0, a1, a2 );

	if( temp3 < 0 ){
		printf( "CD Seek Error(%x)\n", temp3 );
		return temp3;
		return; // SYNTAX NOTICE
	}
	return a0;
}

/*---------------------------------------------------------------------------*/

int PcmClose( int a0 )
{
	int temp3 = 0;

	if( pak_cd_read_fg ){
		if( !pak_read_fg ){
			cdClose();
		}
		return temp3;
	}
	if( !pak_read_fg ){
		temp3 = pcClose( a0 );
		if( temp3 < 0 ){
			printf( "PcmClose Error(%x)\n", temp3 );
		}
	}
	return temp3;
}

/*---------------------------------------------------------------------------*/

int EEOpen( int a0 )
{
	u_int temp, *temp2;

	if( (a0 & 0xFF000000) == 0xF5000000 ){
		temp = 1;
	} else if( (a0 & 0xFF000000) == 0xF4000000 ){
		temp = 2;
	}
	else {
		//
		// EMPTY BLOCK
		//
	}
	temp2 = (pak_header+0x01FA)+(6*temp);

	return temp;
}

/*---------------------------------------------------------------------------*/

int EERead( u_int a0, u_int *a1, u_int a2, u_int a3 )
{
	u_int *temp3;
	struct unkstr24 *temp2;
	u_int temp = 0;

	if( a0 != 1 && a0 != 2 ){
		str2_iop_load_set[a0-1] = 0;
		return 0;
	}

	temp2 = &(((struct unkstr24 *)(pak_header+506))[a0]);

	if( temp2->unk0C <= (u_int)temp2->unk10 ){
		str2_iop_load_set[a0-1] = 0;
		return 0;
	}

	temp3 = sif_get_mem(a1, temp2->unk04 << 4, a3);

	while( 1 ){
		DelayThread( 0x2710 );
		if( temp3[0] & 0x80000000 ){
			break;
		}
		temp++;
		if( !(temp & 0xFFFF) ){
			//
			// EMPTY BLOCK
			//
		}
	}

	if( temp2->unk0C ){
		// the triple load here is in the wrong order
		// should be v1, v0, v1
		// is v0, v0, v1
		temp2->unk10++;
	}

	sif_rv_release_queue( temp3 );
	str2_iop_load_set[a0-1] = 0;
	return a3;
}
