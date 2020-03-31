/*
 * Sound Driver for PS2 IOP
 * Stream System Module 1
 */
/* ver."ZONE OF THE ENDERS"
 */
#include <sys/types.h>
#include <kernel.h>
#include <libsd.h>

#include "sd_incl.h"
#include "sd_ext.h"

/*---------------------------------------------------------------------------*/

/* unreferenced TGS2000 variables */
// u_int str_mute_fg;
// u_int ee_str_load_addr;

unsigned int str_mono_offset;
unsigned int str_counter;
unsigned int str_wait_fg;
unsigned int str_wave_size;
unsigned short str_volume;
unsigned int str_counter_low;
unsigned int str_mono_fg;
unsigned int str_next_idx;
unsigned int spu_str_idx;
int str_off_ctr;
unsigned int str_status;
unsigned int spu_str_start_ptr_r;
unsigned int str_trans_offset;
unsigned int str_unplay_size;
unsigned int mute_str_l_r_fg;
int str_fp;
unsigned int str_unload_size;
unsigned int str_keyoffs;
unsigned int str_first_load;
unsigned int spu_str_start_ptr_l;
unsigned int str_play_offset;
unsigned int str_load_code;
unsigned short str_pitch;
unsigned int str_l_r_fg;
unsigned int str_play_idx;
unsigned int str_stop_fg;
unsigned int str_start_offset;
unsigned char *str_trans_buf;
unsigned int str_read_idx;

u_char str_header[0x8800];
u_int str_read_status[8];

/*---------------------------------------------------------------------------*/

void str_tr_off( void )
{
	str_keyoffs = 0x30000000;
}

/*---------------------------------------------------------------------------*/

void str_spuwr( void )
{
	if( str_keyoffs ){
		sceSdSetSwitch( SD_CORE_1|SD_S_KOFF, str_keyoffs );
		str_keyoffs = 0;
	}
}

/*---------------------------------------------------------------------------*/

int StartStream1( void )
{
	int temp, i;
	
	if( str_fp ){
		str_tr_off();
		PcmClose( str_fp );
		str_fp = 0;
	}
	
	str_fp = PcmOpen( str_load_code, 1 );
	
	if( str_fp < 0 ){
		str_load_code = 0;
		str_first_load = 0;
		str_fp = 0;
		return -1;
	}
	
	temp = PcmRead( str_fp, str_header, 0x8800 );
	
	str_wave_size =  str_header[0] << 24;
	str_wave_size |= str_header[1] << 16;
	str_wave_size |= str_header[2] << 8;
	str_wave_size |= str_header[3];
	
	str_unplay_size = str_unload_size = str_wave_size;
	
	str_volume =  str_header[4] << 8;
	str_volume |= str_header[5];
	
	str_pitch =  str_header[6] << 8;
	str_pitch |= str_header[7];
	str_pitch = (str_pitch * 4096u) / 48000u;
	//~ str_pitch *= 4096;
	//~ str_pitch /= 48000;
	
	if( str_header[8] == 1 ){
		str_mono_fg = 1;
	} else {
		str_mono_fg = 0;
	}
	
	str_trans_buf = str_header+0x0800;
	temp -= 0x0800;
	
	if( temp < str_unload_size ){
		str_unload_size -= temp;
	} else {
		str_unload_size = 0;
	}
	
	str_trans_offset = 0;
	
	for( i = 0 ; i < 8 ; i++ ){
		str_read_status[i] = 1;
	}
	
	str_read_idx = 0;
	return 0;
}

/*---------------------------------------------------------------------------*/

int StartStream2( void )
{
	int temp, i;
	
	if( str_fp ){
		str_tr_off();
		PcmClose( str_fp );
		str_fp = 0;
	}
	
	str_fp = PcmOpen( str_load_code, 1 );
	
	if( str_fp < 0 ){
		str_load_code = 0;
		str_first_load = 0;
		str_fp = 0;
		return -1;
	}
	
	temp = PcmRead( str_fp, str_header, 0x0800 );
	
	str_wave_size =  str_header[0] << 24;
	str_wave_size |= str_header[1] << 16;
	str_wave_size |= str_header[2] << 8;
	str_wave_size |= str_header[3];
	
	if( str_start_offset*0x1000 >= str_wave_size ){
		PcmClose( str_fp );
		str_load_code = 0;
		str_first_load = 0;
		str_fp = 0;
		return -1;
	} else {
		//
		// EMPTY BLOCK
		//
	}

	str_wave_size -= str_start_offset*0x1000;
	str_unplay_size = str_unload_size = str_wave_size;
	str_volume = str_header[4] << 8;
	str_volume |= str_header[5];
	str_pitch = str_header[6] << 8;
	str_pitch |= str_header[7];
	str_pitch = (u_int)(str_pitch * 4096) / 48000;
	
	if( str_header[8] == 1 ){
		str_mono_fg = 1;
	} else {
		str_mono_fg = 0;
	}
	
	PcmLseek( str_fp, str_start_offset*0x1000, 1 );
	str_trans_buf = str_header+0x800;
	temp = PcmRead( str_fp, str_trans_buf, 0x8000 );
	
	if( str_unload_size > temp ){
		str_unload_size -= temp;
	} else {
		str_unload_size = 0;
	}
	
	str_trans_offset = 0;
	
	for( i = 0 ; i < 8 ; i++) {
		str_read_status[i] = 1;
	}
	
	str_read_idx = 0;
	return 0;
}

/*---------------------------------------------------------------------------*/

int StartStream( void )
{
	if( str_start_offset ){
		return StartStream2();
	} else {
		return StartStream1();
	}
}

/*---------------------------------------------------------------------------*/

void StrCdLoad( void )
{
	int temp, i, j, temp4;
	
	if( str_status < 3 || str_status > 5 ){
		return;
	}
	for( i = 0 ; i < 2 ; i++ ){
		if( str_unload_size ){
			temp4 = 0;
			for( j = 0 ; j < 4 ; j++ ){
				temp4 |= str_read_status[str_read_idx+j];
			}
			if( !temp4 ){
				
				// Wait for 8 V-blanks
				WaitVblankEnd(); // 1st interval (end-only)
				WaitVblankStart(); WaitVblankEnd(); // 2nd interval
				WaitVblankStart(); WaitVblankEnd(); // 3rd interval
				WaitVblankStart(); WaitVblankEnd(); // 4th interval
				WaitVblankStart(); WaitVblankEnd(); // 5th interval
				WaitVblankStart(); WaitVblankEnd(); // 6th interval
				WaitVblankStart(); WaitVblankEnd(); // 7th interval
				WaitVblankStart(); WaitVblankEnd(); // 8th interval
				
				if( str_unload_size > 0x4000 ){
					temp = PcmRead( str_fp, str_trans_buf+str_trans_offset, 0x4000 );
					for( j = 0 ; j < 4 ; j++ ){
						str_read_status[str_read_idx+j] = 1;
					}
					str_read_idx += 4;
					if( str_read_idx == 8 ){
						str_read_idx = 0;
					}
					str_unload_size -= 0x4000;
					str_trans_offset += 0x4000;
					if( str_trans_offset >= 0x8000 ){
						str_trans_offset = 0;
					}
				} else {
					temp = PcmRead( str_fp, str_trans_buf+str_trans_offset, 0x4000 );
					for( j = 0 ; j < 4 ; j++ ){
						str_read_status[str_read_idx+j] = 1;
					}
					str_read_idx += 4;
					if( str_read_idx == 8 ){
						str_read_idx = 0;
					}
					str_unload_size = 0;
					break;
				}
			}
		}
	}
}

/*---------------------------------------------------------------------------*/

void str_load( void )
{
	switch( str_status-1 ){
	case 0:
		if( StartStream() ){
			str_status = 0;
		} else {
			str_l_r_fg = 0;
			str_status = 2;
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		StrCdLoad();
		break;
	case 5:
		break;
	case 6:
		PcmClose( str_fp );
		str_load_code = 0;
		str_status = 0;
		str_fp = 0;
		break;
	}
}

/*---------------------------------------------------------------------------*/

int StrSpuTrans( void )
{
	int temp = 0, temp2 = 0;
	
	if( str_stop_fg && str_status > 1 ){
		switch( str_stop_fg ){
		case 1:
			sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_ADSR1, 0x00FF );
			sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_ADSR2, 0x0007 );
			sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_ADSR1, 0x00FF );
			sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_ADSR2, 0x0007 );
			str_status = 7;
			break;
		
		case 2:
			sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_ADSR1, 0x00FF );
			sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_ADSR2, 0x000D );
			sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_ADSR1, 0x00FF );
			sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_ADSR2, 0x000D );
			break;
		}
		str_tr_off();
		str_stop_fg = 0;
	}

/* ///////////////////////////////////////////////////////////////////////// */
	switch( str_status-2 ){
	case 0:
		if( !str_l_r_fg ){
			str_play_idx = 0;
			str_play_offset = 0;
			spu_str_start_ptr_l = 0x5020;
			sceSdSetAddr( SD_CORE_1|SD_VOICE_20|SD_VA_LSAX, 0x5020 );
			sceSdVoiceTrans(
				1,                                   /* transfer channel */
				SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
				str_trans_buf,                       /* IOP memory addr  */
				(u_char *)spu_str_start_ptr_l,       /* SPU memory addr  */
				0x0800 );                            /* transfer size    */
			if( !str_mono_fg ){
				str_play_offset = 0x0800;
				str_unplay_size -= 0x0800;
			}
			str_l_r_fg = 1;
		} else {
			spu_str_start_ptr_l = 0x6020;
			sceSdSetAddr( SD_CORE_1|SD_VOICE_21|SD_VA_LSAX, 0x6020 );
			sceSdVoiceTrans(
				1,                                   /* transfer channel */
				SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
				str_trans_buf+str_play_offset,       /* IOP memory addr  */
				(u_char *)spu_str_start_ptr_r,       /* SPU memory addr  */
				0x0800 );                            /* transfer size    */
			str_play_offset += 0x0800;
			str_unplay_size -= 0x0800;
			if( !str_mono_fg ){
				str_read_status[str_play_idx] = 0;
				str_play_idx++;
			}
			// FIXME?: The assembly for this appears weird when diffed, could be the fault of diff though
			str_l_r_fg = 0;
			str_status++;
		}
		temp = 1;
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 1:
		if( !str_unplay_size || (str_unplay_size & 0x80000000) ){
			str_status++;
		}
		if( !str_l_r_fg ){
			str_trans_buf[str_play_offset+0x07F1] = str_trans_buf[str_play_offset+0x07F1] | 1;
			sceSdVoiceTrans(
				1,                                   /* transfer channel */
				SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
				str_trans_buf+str_play_offset,       /* IOP memory addr  */
				(u_char *)spu_str_start_ptr_l+0x800, /* SPU memory addr  */
				0x0800 );                            /* transfer size    */
			if( !str_mono_fg ){
				str_play_offset += 0x0800;
				str_unplay_size -= 0x0800;
			}
			str_l_r_fg = 1;
		} else {
			str_trans_buf[str_play_offset+0x07F1] = str_trans_buf[str_play_offset+0x07F1] | 1;
			sceSdVoiceTrans(
				1,                                   /* transfer channel */
				SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
				str_trans_buf+str_play_offset,       /* IOP memory addr  */
				(u_char *)spu_str_start_ptr_r+0x800, /* SPU memory addr  */
				0x0800 );                            /* transfer size    */
			str_play_offset += 0x0800;
			str_unplay_size -= 0x0800;
			str_read_status[str_play_idx] = 0;
			str_play_idx++;
			str_l_r_fg = 0;
			str_status++;
		}
		temp = 1;
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 2:
		if( str_first_load ){
			str_first_load = 0;
		}
		if( str_wait_fg ){
			break;
		}
		if( sound_mono_fg ){
			sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_VOLL, (((str_volume * se_pant[0x80]) >> 7) * str_master_vol) / 0x3FFF );
			sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_VOLR, (((str_volume * se_pant[0x80]) >> 7) * str_master_vol) / 0x3FFF );
		} else {
			sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_VOLL, (((str_volume * se_pant[0xFC]) >> 7) * str_master_vol) / 0x3FFF );
			sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_VOLR, 0 );
		}
		sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_PITCH, (u_short)((u_int)(str_pitch*str_master_pitch) >> 12) );
		sceSdSetAddr( SD_CORE_1|SD_VOICE_20|SD_VA_SSA, 0x5020 );
		sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_ADSR1, 0x00FF );
		sceSdSetParam( SD_CORE_1|SD_VOICE_20|SD_VP_ADSR2, 0x0007 );
		if( sound_mono_fg ){
			sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_VOLL, (((str_volume*se_pant[0x80]) >> 7)*str_master_vol) / 0x3FFF );
			sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_VOLR, (((str_volume*se_pant[0x80]) >> 7)*str_master_vol) / 0x3FFF );
		} else {
			sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_VOLL, 0 );
			sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_VOLR, (((str_volume*se_pant[0xFC]) >> 7)*str_master_vol) / 0x3FFF );
		}
		sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_PITCH, (u_short)((u_int)(str_pitch*str_master_pitch) >> 12) );
		sceSdSetAddr( SD_CORE_1|SD_VOICE_21|SD_VA_SSA, 0x6020 );
		sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_ADSR1, 0x00FF );
		sceSdSetParam( SD_CORE_1|SD_VOICE_21|SD_VP_ADSR2, 0x0007 );
		sceSdSetSwitch( SD_CORE_1|SD_S_KON, 0x30 );
		mute_str_l_r_fg = 0;
		spu_str_idx = 0;
		str_next_idx = 0x0800;
		str_status++;
		if( !str_unplay_size || (str_unplay_size & 0x80000000) ){
			str_off_ctr = 0x1F;
			str_status++;
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 3:
		if( !sceSdGetParam( SD_CORE_1|SD_VOICE_20|SD_VP_ENVX ) ){
			str_off_ctr = -1;
			str_status++;
			break;
		}
		spu_str_idx = sceSdGetAddr( SD_CORE_1|SD_VOICE_20|SD_VA_NAX );
		spu_str_idx -= 0x5020;
		if( !(spu_str_idx < 0x1000) || (spu_str_idx & 0x80000000) ){
			break;
		}
		if( str_counter_low > spu_str_idx ){
			str_counter += 0x1000;
		}
		str_counter_low = spu_str_idx;
		if( (str_next_idx == (spu_str_idx & 0x0800)) || str_l_r_fg || mute_str_l_r_fg ){
			temp = 1;
			if( str_read_status[str_play_idx] && !mute_str_l_r_fg ){
				if( str_mono_fg ){
					str_mono_offset = 0;
				}
				if( spu_str_idx >= 0x0800 ){
					if( !str_l_r_fg ){
						sceSdVoiceTrans(
							1,                                   /* transfer channel */
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
							str_trans_buf+str_play_offset,       /* IOP memory addr  */
							(u_char *)spu_str_start_ptr_l,       /* SPU memory addr  */
							0x0800 );                            /* transfer size    */
						str_l_r_fg = 1;
					} else {
						sceSdVoiceTrans(
							1,                                   /* transfer channel */
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
							str_trans_buf+str_play_offset,       /* IOP memory addr  */
							(u_char *)spu_str_start_ptr_r,       /* SPU memory addr  */
							0x0800 );                            /* transfer size    */
						str_next_idx = (str_next_idx+0x0800) & 0x0FFF;
						str_l_r_fg = 0;
						if( !str_mono_fg ){
							str_read_status[str_play_idx] = 0;
							str_play_idx++;
							if( str_play_idx == 8 ){
								str_play_idx = 0;
							}
						}
						if( str_mono_fg ){
							str_mono_offset = 1;
						}
					}
				} else {
					str_trans_buf[str_play_offset+0x07F1] = str_trans_buf[str_play_offset+0x07F1] | 1;
					if( !str_l_r_fg ){
						sceSdVoiceTrans(
							1,                                   /* transfer channel */
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
							str_trans_buf+str_play_offset,       /* IOP memory addr  */
							(u_char *)spu_str_start_ptr_l+0x800, /* SPU memory addr  */
							0x0800 );                            /* transfer size    */
						str_l_r_fg = 1;
					} else {
						str_next_idx = (str_next_idx+0x0800) & 0x0FFF;
						sceSdVoiceTrans(
							1,                                   /* transfer channel */
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
							str_trans_buf+str_play_offset,       /* IOP memory addr  */
							(u_char *)spu_str_start_ptr_r+0x800, /* SPU memory addr  */
							0x0800 );                            /* transfer size    */
						str_l_r_fg = 0;
						str_read_status[str_play_idx] = 0;
						str_play_idx++;
						if( str_play_idx == 8 ){
							str_play_idx = 0;
						}
						if( str_mono_fg ){
							str_mono_offset = 1;
						}
					}
				}
				if( str_mono_fg ) {
					if( str_mono_offset ){
						str_play_offset += 0x0800;
						if( str_play_offset == 0x8000 ){
							str_play_offset = 0;
						}
						if( str_unplay_size > 0x0800 ){
							str_unplay_size -= 0x0800;
						} else {
							str_off_ctr = 0x1F;
							str_play_offset = 0;
							str_status++;
						}
					}
				} else {
					str_play_offset += 0x0800;
					if( str_play_offset == 0x8000 ){
						str_play_offset = 0;
					}
					if( str_unplay_size > 0x0800 ){
						str_unplay_size -= 0x0800;
					} else {
						str_off_ctr = 0x1F;
						str_play_offset = 0;
						str_status++;
					}
				}
			} else {
				if( spu_str_idx >= 0x0800 ){
					dummy_data[1] = 6;
					dummy_data[0x07F1] = 2;
					if( !mute_str_l_r_fg ){
						sceSdVoiceTrans(
							1,                                   /* transfer channel */
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
							dummy_data,                          /* IOP memory addr  */
							(u_char *)spu_str_start_ptr_l,       /* SPU memory addr  */
							0x0800 );                            /* transfer size    */
						mute_str_l_r_fg = 1;
					} else {
						sceSdVoiceTrans(
							1,                                   /* transfer channel */
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
							dummy_data,                          /* IOP memory addr  */
							(u_char *)spu_str_start_ptr_r,       /* SPU memory addr  */
							0x0800 );                            /* transfer size    */
						str_next_idx = (str_next_idx+0x0800) & 0x0FFF;
						mute_str_l_r_fg = 0;
					}
				} else {
					dummy_data[1] = 2;
					dummy_data[0x07F1] = 3;
					if( !mute_str_l_r_fg ){
						mute_str_l_r_fg = 1;
						sceSdVoiceTrans(
							1,                                   /* transfer channel */
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
							dummy_data,                          /* IOP memory addr  */
							(u_char *)spu_str_start_ptr_l+0x800, /* SPU memory addr  */
							0x0800 );                            /* transfer size    */
					} else {
						mute_str_l_r_fg = 0;
						str_next_idx = (str_next_idx+0x800) & 0x0FFF;
						sceSdVoiceTrans(
							1,                                   /* transfer channel */
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA, /* transfer mode    */
							dummy_data,                          /* IOP memory addr  */
							(u_char *)spu_str_start_ptr_r+0x800, /* SPU memory addr  */
							0x0800 );                            /* transfer size    */
					}
				}
			}
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 4:
		str_counter_low += 0x80;
		if( str_counter_low >= 0x1000 ){
			str_counter += 0x1000;
			str_counter_low &= 0x0FFF;
		}
		if( --str_off_ctr == -2 ){
			str_tr_off();
			str_status++;
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 5:
		str_counter_low += 0x80;
		if( str_counter_low >= 0x1000 ){
			str_counter += 0x1000;
			str_counter_low &= 0x0FFF;
		}
		temp2 = 1;
		break;
	}
	return (temp | temp2);
}

/*---------------------------------------------------------------------------*/

void str_int( void )
{
	if( sceSdVoiceTransStatus( 1, SD_TRANS_STATUS_CHECK ) == 1 && StrSpuTrans() ){
		WakeupThread( id_SdMain );
	}
	str_spuwr();
}
