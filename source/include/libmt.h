/*
 * LibMT C++ Version (for PlayStation 2)
 * Copyright (C) Konami Computer Entertainment Japan
 * Codebase Restoration by Joy Division
 */
/* ver."ZONE OF THE ENDERS"
 */
#ifndef INCLUDED_LIBMT_H
#define INCLUDED_LIBMT_H

#include "global.h"
#include "zoe_defs.h"
#include "zoe_types.h"

#include "def_mot.h"
#include "libalg.h"
//#include "libdg.h"

// external
class DG_MDL;
class DG_OBJ;

/*---------------------------------------------------------------------------*/

// ref.default.pdb
class MT_NODE
{
public: //! check modifier
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
public: //! check modifier
	sint32      nMot;
	DEF_MOTION* pMot;
	uint16      u16StreamFlag;
	uint16      u16Counter;
	uint16      u16Type;
	uint8       u8Level;
	uint8       u8Next;
	float       fspdPlay;
	float       fspdInt;

public:
	void Init(DEF_MOTION_LIST*);
};

/*---------------------------------------------------------------------------*/

// ref.default.pdb
class MT_STREAM
{
public: //! check modifier
	float           fFrame;
	float           fdFrame;
	uint16          u16State;
	float           fRate;
	float           fdRate;
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
public: //! check modifier
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
public: //! check modifier
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
public: //! check modifier
	MT_MOTION* pmt;

public:
	MT_MOTPTR();
	~MT_MOTPTR();

	MT_MOTION* operator MT_MOTION*();
	MT_MOTION* operator->();

	void Destroy();
};

/*---------------------------------------------------------------------------*
 * Prototypes
 *---------------------------------------------------------------------------*/

/* Motion Manager (motman.c) */
void MT_Init();

#endif /* END OF FILE */
