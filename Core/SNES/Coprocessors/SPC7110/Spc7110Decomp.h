#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"

//Based on bsnes' code (by byuu)
//original implementation: neviksti
//optimized implementation: talarubi

class Spc7110;

class Spc7110Decomp : public ISerializable
{
private:
	enum : uint32_t { MPS = 0, LPS = 1 };
	enum : uint32_t { One = 0xAA, Half = 0x55, Max = 0xFF };

	struct ModelState
	{
		uint8_t probability;  //of the more probable symbol (MPS)
		uint8_t next[2];      //next state after output {MPS, LPS}
	};
	static ModelState evolution[53];

	struct Context
	{
		uint8_t prediction;   //current model state
		uint8_t swap;         //if 1, exchange the role of MPS and LPS
	};	
	Context _context[5][15];     //not all 75 contexts exists; this simplifies the code

	Spc7110* _spc;

	uint32_t _bpp;             //bits per pixel (1bpp = 1; 2bpp = 2; 4bpp = 4)
	uint32_t _offset;          //SPC7110 data ROM read offset
	uint32_t _bits;            //bits remaining in input
	uint16_t _range;         //arithmetic range: technically 8-bits, but Max+1 = 256
	uint16_t _input;         //input data from SPC7110 data ROM
	uint8_t _output;
	uint64_t _pixels;
	uint64_t _colormap;      //most recently used list
	uint32_t _result;        //decompressed word after calling decode()

private:
	uint8_t ReadByte();
	uint32_t Deinterleave(uint64_t data, uint32_t bits);
	uint64_t MoveToFront(uint64_t list, uint32_t nibble);

public:
	Spc7110Decomp(Spc7110* spc);
	virtual ~Spc7110Decomp();

	void Initialize(uint32_t mode, uint32_t origin);
	void Decode();
	uint32_t GetResult();
	uint8_t GetBpp();

	void Serialize(Serializer& s) override;
};