#ifndef INCLUDED_LIBSTRM_H
#define INCLUDED_LIBSTRM_H

#include <libmpeg.h>

#include "global.h"
#include "zoe_defs.h"
#include "zoe_types.h"

// internal
class STRM_RING_AREA;
class STRM_RING_BUFFER;
class STRM_DATACONSUMER;

//=============================================================================
// Stream Common Defines
//=============================================================================

enum {
/* 0x0 */ STRM_DATANUM_JIMAKU,			// 字幕 // Subtitles
/* 0x1 */ STRM_DATANUM_PCM,				// VAG ADPCM
/* 0x2 */ STRM_DATANUM_PROGDEMO,		// Program Demo
/* 0x3 */ STRM_DATANUM_POLYDEMO,		// Polygon Demo
/* 0x4 */ STRM_DATANUM_SCN,				// Scenario
/* 0x5 */ STRM_DATANUM_DMX,				// DMX
/* 0x6 */ STRM_DATANUM_MPG,				// MPEG2
/* 0x7 */ STRM_DATANUM_EU_JIMAKU,		// 字幕（ヨーロッパ語族）
/* 0x7 */ STRM_DATANUM_JIMAKU_ENGLISH = STRM_DATANUM_EU_JIMAKU,
/* 0x8 */ STRM_DATANUM_JIMAKU_FRENCH,
/* 0x9 */ STRM_DATANUM_JIMAKU_GERMAN,
/* 0xA */ STRM_DATANUM_JIMAKU_ITALIAN,
/* 0xB */ STRM_DATANUM_JIMAKU_SPANISH,
/* 0xC */ STRM_DATANUM_JIMAKU_PORTUGUESE,
/* 0xD */ STRM_DATANUM_MAX
};

//=============================================================================
// Stream Ring Buffer
//=============================================================================

class STRM_RING_AREA
{
public: //! check modifier
	int32	indxTop;
	int32	size;
	UINT8*	pBufTop;
	int32	sizeBuf;

public:
	int32	Write( STRM_RING_AREA* );
	int32	Write( void*, int32 );
	void*	GetTop();
};

/*---------------------------------------------------------------------------*/

class STRM_RING_BUFFER
{
public: //! check modifier
	void*	pBufTop;
	int32	sizeBuf;
	int32	indxDataTop;
	int32	sizeData;
	int32	indxFreeTop;
	int32	sizeFree;

public:
	void	Init( void*, int32 );
	int32	GetFree( STRM_RING_AREA*, int32 );
	int32	GetFree( STRM_RING_AREA* );
	void	UngetFree( int32 );
	void	PutData( int32 );
	int32	GetData( STRM_RING_AREA*, int32 );
	int32	GetData( STRM_RING_AREA* );
	void	UngetData( int32 );
	void	PutFree( int32 );
	void*	GetTop();
	int32	GetBufferSize();
	void	SetArea( STRM_RING_AREA*, UINT8*, int32 );
};

//=============================================================================
// Stream Data Consumer
//=============================================================================

class STRM_DATACONSUMER : public STRM_RING_BUFFER
{
public: //! check modifier

	typedef STRM_DATACONSUMER* (*NEWFUNC)(sceMpeg*);

	enum PHASE {
		INIT1,		//
		INIT2,		//
		INIT3,		//
		INIT4,		//
		INIT5,		//
		PRIMED,		//
		ACT,		//
		ACT2,		//
		ACT3,		//
		ACT4,		//
		END			//
	};

	static const int32 TIMEOUT_INIT = DEFINE_ME;

	void*				pBuf;
	STRM_RING_BUFFER*	pRingRead;
	int32				sizeExtend;
	PHASE				ph;
	BOOL32				bPackMem;
	int32				ctrInitTimeout;

public:
	int32 Update( int32 );

	STRM_DATACONSUMER( const STRM_DATACONSUMER& );
	STRM_DATACONSUMER( int32, int32, int32 );
	~STRM_DATACONSUMER();

	void	Cancel();
	void	SetExtendArea();
	void	CheckInitTimeout();
	int32	CbData( sceMpeg*, sceMpegCbDataStr* );

public: //! check modifier
	static void (*pNewfunc[13])(sceMpeg*);
	static char*	pnmFile;
	static BOOL32	bStrEnd;

public:
	static void SetFunc( int32, void(*)(sceMpeg*) );

	STRM_DATACONSUMER& operator = ( const STRM_DATACONSUMER& );
};

//=============================================================================
// Global Functions
//=============================================================================

/* streamcd.cc */
void STRM_LoadPcmDir();

#endif /* END OF FILE */
