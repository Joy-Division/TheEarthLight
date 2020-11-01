#include <sys/types.h>
#include <kernel.h>
#include <libsd.h>

#include "sd_incl.h"
#include "sd_ext.h"
#include "sd_debug.h"

/*---------------------------------------------------------------------------*/

unsigned short str2_master_vol = 0x3FFF;
unsigned short str2_master_pitch = 0x1000;

unsigned int spu_str2_start_ptr_r[2];
unsigned int str2_iop_load_set[2];
unsigned int str2_wait_fg[2];
unsigned int str2_first_load[2];
unsigned int spu_str2_start_ptr_l[2];
unsigned int str2_off_ctr[2];
int str2_fp[2];
unsigned int str2_mono_fg[2];
unsigned short str2_volume[2];
unsigned int str2_l_r_fg[2];
unsigned int spu_str2_idx[2];
unsigned int str2_load_code[2];
unsigned int ee2_buf_idx[2];
unsigned int str2_play_offset[2];
unsigned int str2_stop_fg[2];
short str2_pitch[2];
unsigned int str2_unload_size[2];
unsigned int str2_mono_offset[2];
unsigned int str2_unplay_size[2];
unsigned int str2_next_idx[2];
int mute2_l_r_fg[2];
unsigned int str2_play_counter[2];
unsigned int str2_keyoffs;
unsigned int str2_mute_fg[2];
unsigned int str2_counter[2];
unsigned int str2_status[2];
unsigned char *str2_trans_buf[2];
unsigned int str2_play_idx[2];
unsigned int str2_read_disable[2];

u_char eeload2_buf[0x00010000];
u_int str2_read_status[2][8];

/*---------------------------------------------------------------------------*/

void str2_tr_off( u_int a0 )
{
	if( !a0 ){
		str2_keyoffs |= 0x300000;
	} else {
		str2_keyoffs |= 0xC00000;
	}
}

/*---------------------------------------------------------------------------*/

void str2_spuwr( void )
{
	if( str2_keyoffs ){
		sceSdSetSwitch( SD_CORE_1|SD_S_KOFF, str2_keyoffs );
		str2_keyoffs = 0;
	}
}

/*---------------------------------------------------------------------------*/

int StartEEStream( u_int a0 )
{
	int temp, i;

	str2_read_disable[a0] = 0;
	str2_tr_off( a0 );

	if( (str2_fp[a0] = EEOpen(str2_load_code[a0])) < 0 ){
		PRINTF(( "StartEEStream:File Open Error(%x)\n", str2_load_code[a0] ));
		str2_fp[a0] = str2_first_load[a0] = str2_load_code[a0] = 0;
		str2_iop_load_set[a0] = 0;
		return -1;
	}

	str2_unplay_size[a0] = str2_unload_size[a0] = ee_addr[a0].unk14;
	str2_volume[a0] = 0x7F;
	ee2_buf_idx[a0] = 0;
	temp = EERead( str2_fp[a0], (uint *)(a0*0x8000+eeload2_buf+ee2_buf_idx[a0]*0x4000), ee2_buf_idx[a0], 0x4000 );

	for( i = 0 ; i < 4 ; i++ ){
		str2_read_status[a0][ee2_buf_idx[a0]*4+i] = 1;
	}

	ee2_buf_idx[a0]++;

	if( str2_unload_size[a0] > temp ){
		str2_unload_size[a0] -= temp;
	} else {
		str2_unload_size[a0] = 0;
	}

	for( i = 0 ; i < 4 ; i++ ){
		str2_read_status[a0][ee2_buf_idx[a0]*4+i] = 0;
	}

	str2_trans_buf[a0] = eeload2_buf+a0*0x8000;
	return 0;
}

/*---------------------------------------------------------------------------*/

void StrEELoad( u_int a0 )
{
	int temp, i, j, temp4;

	for( i = 0 ; i < 2 ; i++ ){
		if( str2_unload_size[a0] ){
			temp4 = 0;
			for( j = 0 ; j < 4 ; j++ ){
				temp4 |= str2_read_status[a0][ee2_buf_idx[a0]*4+j];
			}
			if( !temp4 ){
				// Wait for V-blank
				WaitVblankStart();
				WaitVblankEnd();

				if( str2_unload_size[a0] > 0x4000 ){
					temp = EERead( str2_fp[a0], (uint *)(a0*0x8000+eeload2_buf+ee2_buf_idx[a0]*0x4000), ee2_buf_idx[a0], 0x4000 );
					if( temp ){
						for( j = 0 ; j < 4 ; j++ ){
							str2_read_status[a0][ee2_buf_idx[a0]*4+j] = 1;
						}
						ee2_buf_idx[a0] = (ee2_buf_idx[a0]+1) & 1;
						str2_unload_size[a0] -= 0x4000;
					}
				} else {
					temp = EERead( str2_fp[a0], (uint *)(a0*0x8000+eeload2_buf+ee2_buf_idx[a0]*0x4000), ee2_buf_idx[a0], 0x4000 );
					if( temp ){
						for( j = 0 ; j < 4 ; j++ ){
							str2_read_status[a0][ee2_buf_idx[a0]*4+j] = 1;
						}
						ee2_buf_idx[a0] = (ee2_buf_idx[a0]+1) & 1;
						str2_unload_size[a0] = 0;
					}
					break;
				}
			}
		}
	}
}

/*---------------------------------------------------------------------------*/

void str2_load( void )
{
	int i;

	for( i = 0 ; i < 2 ; i++ ){
		switch( str2_status[i]-1 ){
		case 0:
			if( ee_addr[i].unk0C ){
				if( StartEEStream( i ) ){
					str2_status[i] = 0;
				} else {
					str2_l_r_fg[i] = 0;
					str2_status[i] = 2;
				}
			} else {
				PRINTF(( "Waiting for EE Data Trans.(0)\n" ));
			}
			break;

		case 1:
			if( ee_addr[i].unk0C > 1 ){
				StrEELoad( i );
				str2_status[i]++;
			} else {
				PRINTF(( "Waiting for EE Data Trans.(1)\n" ));
			}

		case 2: /* fallthrough */
		case 3: /* fallthrough */
		case 4: /* fallthrough */
		case 5:
			StrEELoad( i );
			break;

		case 6:
			break;

		case 7:
		#if (defined BORMAN_DEMO || defined DENGEKI_DEMO)
			str2_fp[i] = str2_status[i] = str2_load_code[i] = 0;
			PRINTF(( "***STR Terminate(ch=%x)***\n", i ));
		#else
			ee_addr[i].unk10 = (u_char *)ee_addr[i].unk0C = str2_fp[i] = str2_status[i] = str2_load_code[i] = 0;
		#endif
			break;
		}
	}
}

/*---------------------------------------------------------------------------*/

int Str2SpuTrans( int core )
{
	int temp = 0, temp2 = 0;
	int voll, volr;

#if !(defined BORMAN_DEMO || defined DENGEKI_DEMO)
	if( core >= 2 ){
		return 0;
	}
#endif

	if( str2_stop_fg[core] && str2_status[core] >= 3 ){
		switch( str2_stop_fg[core] ){
		case 1:
			sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_ADSR1, 0x00FF );
			sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_ADSR2, 0x0007 );
			sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_ADSR1, 0x00FF );
			sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_ADSR2, 0x0007 );
		#if !(defined BORMAN_DEMO || defined DENGEKI_DEMO)
			str2_first_load[core] = 0;
		#endif
			str2_status[core] = 8;
			break;

		case 2:
			sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_ADSR1, 0x00FF );
			sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_ADSR2, 0x000D );
			sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_ADSR1, 0x00FF );
			sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_ADSR2, 0x000D );
			break;
		}
		str2_tr_off(core);
		str2_stop_fg[core] = 0;
		PRINTF(( "EE STR Stopped.(ch=%x)\n", core ));
#if !(defined BORMAN_DEMO || defined DENGEKI_DEMO)
	} else if( str2_stop_fg[core] && str2_status[core] ){
		str2_first_load[core] = 0;
		str2_status[core] = 8;
		str2_stop_fg[core] = 0;
#endif
	}

/* ///////////////////////////////////////////////////////////////////////// */
	switch( str2_status[core]-3 ){
	case 0:
		if( !str2_l_r_fg[core] ){
			str2_play_idx[core] = 0;
			str2_play_offset[core] = 0;
			spu_str2_start_ptr_l[core] = core * 0x2000 + 0x5020;
			sceSdSetAddr( SD_CORE_1|(((core*2)+20)<<1)|SD_VA_LSAX, core*0x2000+0x5020 );
			sceSdVoiceTrans(
				1,										// transfer channel
				SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,	// transfer mode
				str2_trans_buf[core],					// IOP memory addr
				(u_char *)(spu_str2_start_ptr_l[core]),	// SPU memory addr
				0x0800									// transfer size
			);
			if( !str2_mono_fg[core] ){
				str2_play_offset[core] = 0x0800;
				str2_unplay_size[core] -= 0x0800;
			}
			str2_l_r_fg[core] = 1;
		} else {
			spu_str2_start_ptr_r[core] = core*0x2000+0x6020;
			sceSdSetAddr( SD_CORE_1|(((core*2)+21)<<1)|SD_VA_LSAX, core*0x2000+0x6020 );
			sceSdVoiceTrans(
				1,												// transfer channel
				SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
				str2_trans_buf[core]+str2_play_offset[core],	// IOP memory addr
				(u_char *)(spu_str2_start_ptr_r[core]),			// SPU memory addr
				0x0800											// transfer size
			);
			str2_play_offset[core] += 0x0800;
			str2_unplay_size[core] -= 0x0800;
			if( !str2_mono_fg[core] ){
				str2_read_status[core][str2_play_idx[core]] = 0;
				str2_play_idx[core]++;
			}
			str2_l_r_fg[core] = 0;
			str2_counter[core] += 0x0800;
			str2_status[core]++;
		}
		temp = 1;
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 1:
		if( !str2_unplay_size[core] || (str2_unplay_size[core] & 0x80000000) ){
			str2_status[core]++;
		}
		if( !str2_l_r_fg[core] ){
			str2_trans_buf[core][str2_play_offset[core]+0x07F1] |= 1;
			sceSdVoiceTrans(
				1,												// transfer channel
				SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
				str2_trans_buf[core]+str2_play_offset[core],	// IOP memory addr
				(u_char*)(spu_str2_start_ptr_l[core])+0x800,	// SPU memory addr
				0x0800											// transfer size
			);
			if( !str2_mono_fg[core] ){
				str2_play_offset[core] += 0x0800;
				str2_unplay_size[core] -= 0x0800;
			}
			str2_l_r_fg[core] = 1;
		} else {
			str2_trans_buf[core][str2_play_offset[core]+0x07F1] |= 1;
			sceSdVoiceTrans(
				1,												// transfer channel
				SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
				str2_trans_buf[core]+str2_play_offset[core],	// IOP memory addr
				(u_char*)(spu_str2_start_ptr_r[core])+0x800,	// SPU memory addr
				0x0800											// transfer size
			);
			str2_play_offset[core] += 0x0800;
			str2_unplay_size[core] -= 0x0800;
			str2_read_status[core][str2_play_idx[core]] = 0;
			str2_play_idx[core]++;
			str2_l_r_fg[core] = 0;
			str2_counter[core] += 0x0800;
			str2_status[core]++;
		}
		temp = 1;
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 2:
		if( str2_first_load[core] ){
			str2_first_load[core] = 0;
		}
		if( str2_wait_fg[core] ){
			break;
		}
		if( sound_mono_fg ){
			voll = volr = (((((unsigned)(str2_volume[core] * vox_fader[core].vol_target) / 0x3F)
							* se_pant[64 - vox_fader[core].pan_target]) / 0x7F) * str2_master_vol)
							/ 0x3FFF;
			sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLL, voll );
			sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLR, volr );
		#ifdef BORMAN_DEMO
			sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLL, (unsigned)voll );
			sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLR, (unsigned)volr );
		#else
			sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLL, (unsigned)voll / 3 );
			sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLR, (unsigned)volr / 3 );
		#endif
			vox_fader[core].vol_current = vox_fader[core].vol_target;
			vox_fader[core].pan_current = vox_fader[core].pan_target;
		} else {
			if( str2_mono_fg[core] ){
				voll = (((((unsigned)(str2_volume[core] * vox_fader[core].vol_target) / 0x3F)
							* se_pant[64 - vox_fader[core].pan_target]) / 0x7F) * str2_master_vol)
							/ 0x3FFF;
				volr = (((((unsigned)(str2_volume[core] * vox_fader[core].vol_target) / 0x3F)
							* se_pant[vox_fader[core].pan_target]) / 0x7F) * str2_master_vol)
							/ 0x3FFF;
				sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLL, voll );
				sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLR, volr );
			#ifdef BORMAN_DEMO
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLL, (unsigned)voll );
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLR, (unsigned)volr );
			#else
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLL, (unsigned)voll / 3 );
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLR, (unsigned)volr / 3 );
			#endif
				vox_fader[core].vol_current = vox_fader[core].vol_target;
				vox_fader[core].pan_current = vox_fader[core].pan_target;
				PRINTF(( "VOX(MONO): voll=%x:volr=%x\n", voll, volr ));
			} else {
			#ifdef BORMAN_DEMO
				sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLL,
					(((unsigned)(str2_volume[core] * se_pant[0x3F] / 0x7F)) * str2_master_vol) / 0x3FFF );
				sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLR, 0 );
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLL, 0 );
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLR,
					(((unsigned)(str2_volume[core] * se_pant[0x3F] / 0x7F)) * str2_master_vol) / 0x3FFF );
			#else
				voll = volr = (((unsigned)(str2_volume[core] * se_pant[0x3F] / 0x7F)) * str2_master_vol) / 0x3FFF;
				sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLL, voll );
				sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLR, 0 );
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLL, 0 );
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLR, volr );
			#endif
			}
		}
		sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_PITCH, (u_int)((u_short)str2_pitch[core]*str2_master_pitch) / 0x1000 );
		sceSdSetAddr( SD_CORE_1|(((core*2)+20)<<1)|SD_VA_SSA, core*0x2000+0x5020 );
		sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_ADSR1, 0x00FF );
		sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_ADSR2, 0x0007 );
		sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_PITCH, (u_int)((u_short)str2_pitch[core]*str2_master_pitch) / 0x1000 );
		sceSdSetAddr( SD_CORE_1|(((core*2)+21)<<1)|SD_VA_SSA, core*0x2000+0x6020 );
		sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_ADSR1, 0x00FF );
		sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_ADSR2, 0x0007 );
		if( !core ){
			sceSdSetSwitch( SD_CORE_1|SD_S_KON, 0x300000 );
		} else {
			sceSdSetSwitch( SD_CORE_1|SD_S_KON, 0xC00000 );
		}
		spu_str2_idx[core] = mute2_l_r_fg[core] = 0;
		str2_next_idx[core] = 0x0800;
		str2_status[core]++;
		if( !str2_unplay_size[core] || (str2_unplay_size[core] & 0x80000000) ){
			PRINTF(( "Str2SpuTrans:status=4 --> 6(%x)\n", str2_unplay_size[core] ));
			str2_off_ctr[core] = 0x1F;
			str2_status[core]++;
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 3:
		if( sceSdGetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_ENVX ) == 0 ){
			str2_off_ctr[core] = -1;
			str2_status[core]++;
			PRINTF(( "Str:Envelope Stopped\n" ));
			break;
		}
		if((vox_fader[core].vol_target != vox_fader[core].vol_current)
		|| (vox_fader[core].pan_target != vox_fader[core].pan_current)){
			if( sound_mono_fg ){
				voll = volr = (((((unsigned)(str2_volume[core] * vox_fader[core].vol_target) / 0x3F)
								* se_pant[0x20]) / 0x7F) * str2_master_vol)
								/ 0x3FFF;
				sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLL, voll );
				sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLR, volr );
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLL, voll );
				sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLR, volr );
				vox_fader[core].vol_current = vox_fader[core].vol_target;
				vox_fader[core].pan_current = vox_fader[core].pan_target;
			} else {
				if( str2_mono_fg[core] ){
					voll = (((((unsigned)(str2_volume[core] * vox_fader[core].vol_target) / 0x3F)
								* se_pant[0x40 - vox_fader[core].pan_target]) / 0x7F) * str2_master_vol)
								/ 0x3FFF;
					volr = (((((unsigned)(str2_volume[core] * vox_fader[core].vol_target) / 0x3F)
								* se_pant[vox_fader[core].pan_target]) / 0x7F) * str2_master_vol)
								/ 0x3FFF;
					sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLL, voll );
					sceSdSetParam( SD_CORE_1|(((core*2)+20)<<1)|SD_VP_VOLR, volr );
					sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLL, voll );
					sceSdSetParam( SD_CORE_1|(((core*2)+21)<<1)|SD_VP_VOLR, volr );
					vox_fader[core].vol_current = vox_fader[core].vol_target;
					vox_fader[core].pan_current = vox_fader[core].pan_target;
				}
			}
		}
		spu_str2_idx[core] = sceSdGetAddr( SD_CORE_1|(((core*2)+20)<<1)|SD_VA_NAX );
		spu_str2_idx[core] -= 0x5020 + core * 0x2000;
		if( spu_str2_idx[core] >= 0x1000 || (spu_str2_idx[core] & 0x80000000) ){
			PRINTF(( "ERROR:MemoryStreamingAddress(%x)\n", spu_str2_idx[core] ));
			break;
		}
		if( !str2_mute_fg[core] ){
			str2_play_counter[core]++;
		}
		if( str2_next_idx[core] == (spu_str2_idx[core] & 0x0800) || str2_l_r_fg[core] || mute2_l_r_fg[core] ){
			temp = 1;
			if( str2_read_status[core][str2_play_idx[core]] && !mute2_l_r_fg[core] ){
				str2_mute_fg[core] = 0;
				if( str2_l_r_fg[core] ){
					str2_counter[core] += 0x0800;
				}
				if( str2_mono_fg[core] ){
					str2_mono_offset[core] = 0;
				}
				if( spu_str2_idx[core] >= 0x0800 ){
					if( !str2_l_r_fg[core] ){
						sceSdVoiceTrans(
							1,												// transfer channel
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
							str2_trans_buf[core]+str2_play_offset[core],	// IOP memory addr
							(u_char *)(spu_str2_start_ptr_l[core]),			// SPU memory addr
							0x0800											// transfer size
						);
						str2_l_r_fg[core] = 1;
					} else {
						sceSdVoiceTrans(
							1,												// transfer channel
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
							str2_trans_buf[core]+str2_play_offset[core],	// IOP memory addr
							(u_char *)(spu_str2_start_ptr_r[core]),			// SPU memory addr
							0x0800											// transfer size
						);
						str2_next_idx[core] = (str2_next_idx[core] + 0x0800) & 0x0FFF;
						str2_l_r_fg[core] = 0;
						if( !str2_mono_fg[core] ){
							str2_read_status[core][str2_play_idx[core]] = 0;
							str2_play_idx[core]++;
							if( str2_play_idx[core] == 8 ){
								str2_play_idx[core] = 0;
							}
						}
						if( str2_mono_fg[core] ){
							str2_mono_offset[core] = 1;
						}
					}
				} else {
					*(str2_trans_buf[core]+str2_play_offset[core]+0x07F1) |= 1;
					if( !str2_l_r_fg[core] ){
						sceSdVoiceTrans(
							1,												// transfer channel
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
							str2_trans_buf[core]+str2_play_offset[core],	// IOP memory addr
							(u_char *)(spu_str2_start_ptr_l[core])+0x800,	// SPU memory addr
							0x0800											// transfer size
						);
						str2_l_r_fg[core] = 1;
					} else {
						str2_next_idx[core] = (str2_next_idx[core] + 0x0800) & 0x0FFF;
						sceSdVoiceTrans(
							1,												// transfer channel
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
							str2_trans_buf[core]+str2_play_offset[core],	// IOP memory addr
							(u_char *)(spu_str2_start_ptr_r[core])+0x800,	// SPU memory addr
							0x0800											// transfer size
						);
						str2_l_r_fg[core] = 0;
						str2_read_status[core][str2_play_idx[core]] = 0;
						str2_play_idx[core]++;
						if( str2_play_idx[core] == 8 ){
							str2_play_idx[core] = 0;
						}
						if( str2_mono_fg[core] ){
							str2_mono_offset[core] = 1;
						}
					}
				}
				if( str2_mono_fg[core] ){
					if( str2_mono_offset[core] ){
						str2_play_offset[core] += 0x0800;
						if( str2_play_offset[core] == 0x8000 ){
							str2_play_offset[core] = 0;
						}
						if( str2_unplay_size[core] > 0x0800 ){
							str2_unplay_size[core] -= 0x0800;
						} else {
							str2_off_ctr[core] = 0x1F;
							str2_play_offset[core] = 0;
							str2_status[core]++;
						}
					}
				} else {
					str2_play_offset[core] += 0x0800;
					if( str2_play_offset[core] == 0x8000 ){
						str2_play_offset[core] = 0;
					}
					if( str2_unplay_size[core] > 0x0800 ){
						str2_unplay_size[core] -= 0x0800;
					} else {
						str2_off_ctr[core] = 0x1F;
						str2_play_offset[core] = 0;
						str2_status[core]++;
					}
				}
			} else {
				PRINTF(( "EE READ Retry(ch=%x)\n", core ));
			#if !(defined BORMAN_DEMO || defined DENGEKI_DEMO)
				str2_unplay_size[core] = str2_unload_size[core];
			#endif
				str2_mute_fg[core] = 1;
				if( spu_str2_idx[core] >= 0x0800 ){
					dummy_data[1] = 6;
					dummy_data[0x07F1] = 2;
					if( !mute2_l_r_fg[core] ){
						sceSdVoiceTrans(
							1,										// transfer channel
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,	// transfer mode
							dummy_data,								// IOP memory addr
							(u_char *)(spu_str2_start_ptr_l[core]),	// SPU memory addr
							0x0800									// transfer size
						);
						mute2_l_r_fg[core] = 1;
					} else {
						sceSdVoiceTrans(
							1,										// transfer channel
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,	// transfer mode
							dummy_data,								// IOP memory addr
							(u_char *)(spu_str2_start_ptr_r[core]),	// SPU memory addr
							0x0800									// transfer size
						);
						str2_next_idx[core] = (str2_next_idx[core] + 0x0800) & 0x0FFF;
						mute2_l_r_fg[core] = 0;
					}
				} else {
					dummy_data[1] = 2;
					dummy_data[0x07F1] = 3;
					if( !mute2_l_r_fg[core] ){
						mute2_l_r_fg[core] = 1;
						sceSdVoiceTrans(
							1,												// transfer channel
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
							dummy_data,										// IOP memory addr
							(u_char *)(spu_str2_start_ptr_l[core])+0x0800,	// SPU memory addr
							0x0800											// transfer size
						);
					} else {
						mute2_l_r_fg[core] = 0;
						str2_next_idx[core] = (str2_next_idx[core] + 0x0800) & 0x0FFF;
						sceSdVoiceTrans(
							1,												// transfer channel
							SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,			// transfer mode
							dummy_data,										// IOP memory addr
							(u_char *)(spu_str2_start_ptr_r[core])+0x0800,	// SPU memory addr
							0x0800											// transfer size
						);
					}
				}
			}
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 4:
		str2_counter[core] += 0x80;
		if( --str2_off_ctr[core] == -2 ){
			str2_tr_off( core );
			str2_status[core]++;
		}
		break;
/* ///////////////////////////////////////////////////////////////////////// */
	case 5:
		str2_counter[core] += 0x80;
		temp2 = 1;
		break;
	}
	return (temp | temp2);
}

/*---------------------------------------------------------------------------*/

void str2_int( void )
{
	if( sceSdVoiceTransStatus( 1, SD_TRANS_STATUS_CHECK ) == 1 ){
		if( Str2SpuTrans( 0 ) ){
			WakeupThread( id_SdEELoad );
		} else if( Str2SpuTrans(1) ){
			WakeupThread( id_SdEELoad );
		}
	}
	str2_spuwr();
}
