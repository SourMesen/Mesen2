#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"

/************************************************************************

S-DD1'algorithm emulation code
------------------------------

Author:	     Andreas Naive
Date:        August 2003
Last update: October 2004

This code is Public Domain. There is no copyright holded by the author.
Said this, the author wish to explicitly emphasize his inalienable moral rights
over this piece of intelectual work and the previous research that made it
possible, as recognized by most of the copyright laws around the world.

This code is provided 'as-is', with no warranty, expressed or implied.
No responsability is assumed by the author in connection with it.

The author is greatly indebted with The Dumper, without whose help and
patience providing him with real S-DD1 data the research would have never been
possible. He also wish to note that in the very beggining of his research,
Neviksti had done some steps in the right direction. By last, the author is
indirectly indebted to all the people that worked and contributed in the
S-DD1 issue in the past.

An algorithm's documentation is available as a separate document.
The implementation is obvious when the algorithm is
understood.

************************************************************************/

class Sdd1Mmc;

class SDD1_IM : public ISerializable
{  //Input Manager
	Sdd1Mmc* _sdd1Mmc;
public:
	SDD1_IM(void) {}
	void prepareDecomp(Sdd1Mmc *mmc, uint32_t readAddr);
	uint8_t getCodeword(const uint8_t code_len);

	void Serialize(Serializer &s) override;

private:
	uint32_t _readAddr;
	uint8_t bit_count;
};

////////////////////////////////////////////////////


class SDD1_GCD
{  //Golomb-Code Decoder

public:
	SDD1_GCD(SDD1_IM *associatedIM);
	void getRunCount(uint8_t code_num, uint8_t *MPScount, bool *LPSind);
	
private:
	SDD1_IM * const IM;

};

//////////////////////////////////////////////////////


class SDD1_BG : public ISerializable
{  // Bits Generator

public:
	SDD1_BG(SDD1_GCD *associatedGCD, uint8_t code);
	void prepareDecomp(void);
	uint8_t getBit(bool *endOfRun);
	
	void Serialize(Serializer &s) override;

private:
	const uint8_t code_num;
	uint8_t MPScount;
	bool LPSind;
	SDD1_GCD *const GCD;

};

////////////////////////////////////////////////


class SDD1_PEM : public ISerializable
{  //Probability Estimation Module

public:
	SDD1_PEM(SDD1_BG *associatedBG0, SDD1_BG *associatedBG1,
		SDD1_BG *associatedBG2, SDD1_BG *associatedBG3,
		SDD1_BG *associatedBG4, SDD1_BG *associatedBG5,
		SDD1_BG *associatedBG6, SDD1_BG *associatedBG7);
	void prepareDecomp(void);
	uint8_t getBit(uint8_t context);
	
	void Serialize(Serializer &s) override;

private:
	struct state
	{
		uint8_t code_num;
		uint8_t nextIfMPS;
		uint8_t nextIfLPS;
	};
	static const state evolution_table[];
	struct SDD1_ContextInfo
	{
		uint8_t status;
		uint8_t MPS;
	} contextInfo[32];
	SDD1_BG * BG[8];

};

///////////////////////////////////////////////////


class SDD1_CM : public ISerializable
{  //Context Model

public:
	SDD1_CM(SDD1_PEM *associatedPEM);
	void prepareDecomp(uint8_t firstByte);
	uint8_t getBit(void);
	
	void Serialize(Serializer &s) override;

private:
	uint8_t bitplanesInfo;
	uint8_t contextBitsInfo;
	uint8_t bit_number;
	uint8_t currBitplane;
	uint16_t prevBitplaneBits[8];
	SDD1_PEM *const PEM;

};

///////////////////////////////////////////////////


class SDD1_OL : public ISerializable
{  //Output Logic

public:
	SDD1_OL(SDD1_CM *associatedCM);
	void prepareDecomp(uint8_t firstByte);
	uint8_t decompressByte();
	
	void Serialize(Serializer &s) override;

private:
	uint8_t bitplanesInfo;
	uint8_t _regs[3];
	SDD1_CM *const CM;

};

class Sdd1Decomp : public ISerializable
{
public:
	Sdd1Decomp();
	void Init(Sdd1Mmc *mmc, uint32_t readAddr);
	uint8_t GetDecompressedByte();

	void Serialize(Serializer &s) override;

private:
	SDD1_IM IM;
	SDD1_GCD GCD;
	SDD1_BG BG0;  SDD1_BG BG1;  SDD1_BG BG2;  SDD1_BG BG3;
	SDD1_BG BG4;  SDD1_BG BG5;  SDD1_BG BG6;  SDD1_BG BG7;
	SDD1_PEM PEM;
	SDD1_CM CM;
	SDD1_OL OL;

};