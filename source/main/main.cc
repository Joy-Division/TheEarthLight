#include <stdlib.h>
#include <string.h>

#include <eekernel.h>
#include <libcdvd.h>
#include <libdev.h>
#include <libdma.h>
#include <libgraph.h>
#include <libmpeg.h>
#include <sifdev.h>
#include <sifrpc.h>

#include "global.h"
#include "zoe_defs.h"
#include "zoe_types.h"

#include "cdbios.h"
#include "mts.h"
#include "sound.h"

#define IN_MAIN_CC // for GV_InitSystem()

#include "../include/libdg.h"
#include "../include/libdm.h"
#include "../include/libev.h"
#include "../include/libfs.h"
#include "../include/libgv.h"
#include "../include/libhzd.h"
#include "../include/libmcard.h"
#include "../include/libscn.h"
#include "../include/libstrm.h"
#include "../include/libtrg.h"
#include "../include/carl.h"

#include "main.h" // for CacheOn()

// TODO: move temporary prototypes
extern void NewInitLoad( char *, int ); /* bgsphere.cc */
extern void NewStartStage(); /* bgsphere.cc */
extern void NewDebugMode(); /* dbgmode.cc */

/*---------------------------------------------------------------------------*/

#if defined(BORMAN_DEMO)
#  define CDVD_MODE SCECdCD
#else
#  define CDVD_MODE SCECdDVD
#endif

static int ReplaceImageFlag = true;

/*---------------------------------------------------------------------------*/
// SLPS99999, status: UNTESTED,   opt: ?
// SLPM65019, status: INCOMPLETE, opt: ?

static void SifLoad()
{
	char *irx_list[] = {
		// SCE IOP modules
		"cdrom0:\\MODULES\\LIBSD.IRX;1",
		"cdrom0:\\MODULES\\SIO2MAN.IRX;1",
		"cdrom0:\\MODULES\\PADMAN.IRX;1",
		"cdrom0:\\MODULES\\MCMAN.IRX;1",
		"cdrom0:\\MODULES\\MCSERV.IRX;1",
		// KCEJ IOP modules
		"cdrom0:\\MODULES\\JSIFMAN.IRX;1",
		"cdrom0:\\MODULES\\CDBIOS.IRX;1",
		"cdrom0:\\MODULES\\NOUSBFS.IRX;1",
		"cdrom0:\\MODULES\\IOP_MAIN.IRX;1"
	};
#if defined(JAPAN_RELEASE)
	//
	// TODO: decompile
	//
#endif
	sceSifInitRpc( 0 );

	for( int i=8 ; i >= 0 ; i-- ){
		printf( "Load module %s\n", irx_list[i] );

		if( sceSifLoadModule( irx_list[i], 0, NULL ) < 0 ){
			printf( "Can't load module %s\n", irx_list[i] );
			exit( 0 );
		}
	}
}

/*---------------------------------------------------------------------------*/
// SLPS99999, status: UNTESTED, opt: ?
// SLPM65019, status: UNTESTED, opt: ?

static void ResetIOP()
{
	char *IopReplaceModule = "cdrom0:\\MODULES\\IOPRP20.IMG;1";

	sceSifInitRpc( 0 );
	sceCdInit( SCECdINIT );
	sceCdMmode( CDVD_MODE );

#if defined(BORMAN_DEMO)
	printf( "IopReplaceModule %s\n", IopReplaceModule );
#endif

	if( ReplaceImageFlag ){
		while( !sceSifRebootIop( IopReplaceModule ) );
		while( !sceSifSyncIop() );
	}

	sceSifInitRpc( 0 );
	sceCdInit( SCECdINIT );
	sceCdMmode( CDVD_MODE );
	sceFsReset();
}

/*---------------------------------------------------------------------------*/
// SLPS99999, status: UNTESTED, opt: ?
// SLPM65019, status: UNTESTED, opt: ?

int main()
{
	sceDevVif0Reset();
	sceDevVif1Reset();
	sceDevVu0Reset();
	sceDevVu1Reset();
	sceGsResetPath();

	CacheOn();

	sceDmaReset( 1 );
	sceGsResetPath();
	sceGsResetGraph( 0, SCE_GS_NOINTERLACE, SCE_GS_NTSC, SCE_GS_FRAME );

	DG_InitClean();

	ResetIOP();	// load IOPRP*.IMG
	SifLoad();	// load IRX modules

	sd_init();

#if defined(BORMAN_DEMO)
	// TODO: Check this, likely NOMATCH
	DEBUG_InitException(
		(((0x00010000 + (int16)0x8000)
		+ (0x00200000 + (int16)0x8000))-16)
	);
#endif

	GV_InitSystem();

	FS_StartDaemon();
	printf( "FS_StartDaemon\n" );

	DG_StartDaemon();
	printf( "DG_StartDaemon\n" );

	SCN_SYSTEM::InitSystem( 200, 200, 200 );

	MC_StartDaemon();
	printf( "MC_StartDaemon\n" );

	EV_StartDaemon();
	printf( "EV_StartDaemon\n" );

	TRG_OBJECT::StartDaemon();
	printf( "TRG_StartDaemon\n" );

	HZD_Init();

	MT_Init();

	DM_MANAGER::InitSystem();

	sceMpegInit();

	DG_DebugPrimDaemon( 40960 );

	cdbios_init();

	FS_CdLoadDirInfo( "\\ZOE.DAT;1", 32 );

	STRM_LoadPcmDir();

#if defined(BORMAN_DEMO)
	NewInitLoad( "init", 0 );
#else
	// TODO: Used by JAPAN_RELEASE; check others.
	// ERROR: Call does not match any candidates.
	// Probably uses an index into a char* array.
	CR_SetStageLoad( "title", 1, "font.pak", 0 );
#endif

	NewStartStage();

#if defined(BORMAN_DEMO)
	NewDebugMode();
#endif

	while( 1 ){
		GV_ACTOR::Execute();
	}
}
