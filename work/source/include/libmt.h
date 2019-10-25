/*
 * [ LibMT ]
 * Motion Processing Library
 *
 * ver.ZONE OF THE ENDERS
 */
#ifndef INC_LIBMT_H
#define INC_LIBMT_H

#include "global.h"
#include "def_mot.h"
#include "libalg.h"
#include "libdg.h"

/*---------------------------------------------------------------------------*
 * Prototypes
 *---------------------------------------------------------------------------*/

/* Motion Manager (motman.c) */
void MT_Init();

/*---------------------------------------------------------------------------*/

// ref.default.pdb
class MT_NODE
{
public: //! unconfirmed modifier
	ALG_QUATER          quat;
	ALG_VECTOR          pos;
	ALG_VECTOR          scl;
	uint16              u16nStream;
	uint16              u16nData;
	DEF_MOTION_NODE*    pmnode;
	uint16              u16PrevFlag;
	uint16              pad1;
	uint32              pad2;

public:
	void Init(uint8);
	void Update(DG_MDL*, MT_STREAM*);
	
	MT_NODE();
	
	MT_NODE& operator=(const MT_NODE&);
};

/*---------------------------------------------------------------------------*/

// ref.default.pdb
class MT_SEQUENCE
{
public: //! unconfirmed modifier
	sint32      nMot;
	DEF_MOTION* pMot;
	uint16      u16StreamFlag;
	uint16      u16Counter;
	uint16      u16Type;
	uint8       u8Level;
	uint8       u8Next;
	float32     fspdPlay;
	float32     fspdInt;

public:
	void Init(DEF_MOTION_LIST*);
};

/*---------------------------------------------------------------------------*/

// ref.default.pdb
class MT_STREAM
{
public: //! unconfirmed modifier
	float32         fFrame;
	float32         fdFrame;
	uint16          u16State;
	float32         fRate;
	float32         fdRate;
	sint16          s16Counter;
	MT_SEQUENCE*    pSeq;
	uint32          pad1;
	uint32          pad2;

public:
	void    Init();
	uint32  Update();
	void    Stop();
};

/*---------------------------------------------------------------------------*/

// ref.default.pdb
class MT_TABLE
{
public: //! unconfirmed modifier
	char*               nmFile;
	DEF_MOTION_LIST*    pmlst;
	sint32              nbNs;
	uint8*              pNs;
	sint32              nbSeq;
	MT_SEQUENCE*        pSeq;
	sint32              pad0;
	sint32              pad1;
	sint32              pad2;

public:
	void Init(DEF_MOTION_LIST*);
};

/*---------------------------------------------------------------------------*/

// ref.default.pdb
class MT_MOTION
{
public: //! unconfirmed modifier
	DG_OBJ*     pobj;
	MT_TABLE*   ptabMot;
	uint32      u32Flag;
	MT_STREAM   stream[2];
	MT_NODE     node[];

public:
	void    Update();
	void    Play(sint32);
	void    Stop(sint32);
	void    Stop();
	void    Destroy();
	sint32  CurrentSeq(sint32);
	
	static MT_MOTION* Init(DG_OBJ*, MT_TABLE*, sint32);
	
	MT_MOTION();
	
	MT_MOTION& operator=(const MT_MOTION&);
};

/*---------------------------------------------------------------------------*/

// ref.default.pdb
class MT_MOTPTR
{
public: //! unconfirmed modifier
	MT_MOTION* pmt;

public:
	MT_MOTPTR();
	~MT_MOTPTR();
	
	MT_MOTION* operator MT_MOTION*();
	MT_MOTION* operator->();
	
	void Destroy();
};

/*---------------------------------------------------------------------------*/
#endif /* END OF FILE */
/*---------------------------------------------------------------------------*/
/* -*- indent-tabs-mode: t; tab-width: 4; mode: cpp; -*- */
/* vim: set noet ts=4 sw=4 ft=cpp ff=dos fenc=euc-jp : */
