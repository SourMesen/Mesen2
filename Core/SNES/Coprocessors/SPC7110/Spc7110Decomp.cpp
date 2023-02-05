#include "pch.h"
#include "SNES/Coprocessors/SPC7110/Spc7110Decomp.h"
#include "SNES/Coprocessors/SPC7110/Spc7110.h"
#include "Utilities/Serializer.h"

//Based on bsnes' code (by byuu)
//original implementation: neviksti
//optimized implementation: talarubi

Spc7110Decomp::Spc7110Decomp(Spc7110* spc)
{
	_spc = spc;
}

Spc7110Decomp::~Spc7110Decomp()
{
}

uint8_t Spc7110Decomp::ReadByte()
{
	return _spc->ReadDataRom(_offset++);
}

//inverse morton code transform: unpack big-endian packed pixels
//returns odd bits in lower half; even bits in upper half
uint32_t Spc7110Decomp::Deinterleave(uint64_t data, uint32_t bits)
{
	data = data & ((1ull << bits) - 1);
	data = 0x5555555555555555ull & (data << bits | data >> 1);
	data = 0x3333333333333333ull & (data | data >> 1);
	data = 0x0f0f0f0f0f0f0f0full & (data | data >> 2);
	data = 0x00ff00ff00ff00ffull & (data | data >> 4);
	data = 0x0000ffff0000ffffull & (data | data >> 8);
	return (uint32_t)(data | data >> 16);
}

//extract a nibble and move it to the low four bits
uint64_t Spc7110Decomp::MoveToFront(uint64_t list, uint32_t nibble)
{
	for(uint64_t n = 0, mask = ~15; n < 64; n += 4, mask <<= 4) {
		if((list >> n & 15) != nibble) {
			continue;
		}
		return (list & mask) + (list << 4 & ~mask) + nibble;
	}
	return list;
}

void Spc7110Decomp::Initialize(uint32_t mode, uint32_t origin)
{
	memset(_context, 0, sizeof(_context));

	_bpp = 1 << mode;
	_offset = origin;
	_bits = 8;
	_range = Max + 1;
	_input = ReadByte();
	_input = _input << 8 | ReadByte();
	_output = 0;
	_pixels = 0;
	_colormap = 0xfedcba9876543210ull;
}

void Spc7110Decomp::Decode()
{
	for(uint32_t pixel = 0; pixel < 8; pixel++) {
		uint64_t map = _colormap;
		uint32_t diff = 0;

		if(_bpp > 1) {
			uint32_t pa = (_bpp == 2 ? (_pixels >> 2) & 3 : (_pixels >> 0) & 15);
			uint32_t pb = (_bpp == 2 ? (_pixels >> 14) & 3 : (_pixels >> 28) & 15);
			uint32_t pc = (_bpp == 2 ? (_pixels >> 16) & 3 : (_pixels >> 32) & 15);

			if(pa != pb || pb != pc) {
				uint32_t match = pa ^ pb ^ pc;
				diff = 4;                        //no match; all pixels differ
				if((match ^ pc) == 0) diff = 3;  //a == b; pixel c differs
				if((match ^ pb) == 0) diff = 2;  //c == a; pixel b differs
				if((match ^ pa) == 0) diff = 1;  //b == c; pixel a differs
			}

			_colormap = MoveToFront(_colormap, pa);

			map = MoveToFront(map, pc);
			map = MoveToFront(map, pb);
			map = MoveToFront(map, pa);
		}

		for(uint32_t plane = 0; plane < _bpp; plane++) {
			uint32_t bit = _bpp > 1 ? 1 << plane : 1 << (pixel & 3);
			uint32_t history = (bit - 1) & _output;
			uint32_t set = 0;

			if(_bpp == 1) {
				set = pixel >= 4;
			} else if(_bpp == 2) {
				set = diff;
			}

			if(plane >= 2 && history <= 1) {
				set = diff;
			}

			auto& ctx = _context[set][bit + history - 1];
			auto& model = evolution[ctx.prediction];
			uint8_t lps_offset = _range - model.probability;
			bool symbol = _input >= (lps_offset << 8);  //test only the MSB

			_output = _output << 1 | ((uint8_t)symbol ^ ctx.swap);

			if(symbol == MPS) {          //[0 ... range-p]
				_range = lps_offset;        //range = range-p
			} else {                     //[range-p+1 ... range]
				_range -= lps_offset;       //range = p-1, with p < 0.75
				_input -= lps_offset << 8;  //therefore, always rescale
			}

			while(_range <= Max / 2) {    //scale back into [0.75 ... 1.5]
				ctx.prediction = model.next[symbol];

				_range <<= 1;
				_input <<= 1;

				if(--_bits == 0) {
					_bits = 8;
					_input += ReadByte();
				}
			}

			if(symbol == LPS && model.probability > Half) {
				ctx.swap ^= 1;
			}
		}

		uint32_t index = _output & ((1 << _bpp) - 1);
		if(_bpp == 1) {
			index ^= _pixels >> 15 & 1;
		}

		_pixels = _pixels << _bpp | (map >> 4 * index & 15);
	}

	if(_bpp == 1) {
		_result = (uint32_t)_pixels;
	} else if(_bpp == 2) {
		_result = Deinterleave(_pixels, 16);
	} else if(_bpp == 4) {
		_result = Deinterleave(Deinterleave(_pixels, 32), 32);
	}
}

uint32_t Spc7110Decomp::GetResult()
{
	return _result;
}

uint8_t Spc7110Decomp::GetBpp()
{
	return _bpp;
}

void Spc7110Decomp::Serialize(Serializer& s)
{
	SV(_bpp); SV(_offset); SV(_bits); SV(_range); SV(_input); SV(_output); SV(_pixels); SV(_colormap); SV(_result);
	for(int i = 0; i < 15; i++) {
		SVI(_context[0][i].swap); SVI(_context[0][i].prediction);
		SVI(_context[1][i].swap); SVI(_context[1][i].prediction);
		SVI(_context[2][i].swap); SVI(_context[2][i].prediction);
		SVI(_context[3][i].swap); SVI(_context[3][i].prediction);
		SVI(_context[4][i].swap); SVI(_context[4][i].prediction);
	}
}

Spc7110Decomp::ModelState Spc7110Decomp::evolution[53] = {
	{0x5a, { 1, 1}}, {0x25, { 2, 6}}, {0x11, { 3, 8}},
	{0x08, { 4,10}}, {0x03, { 5,12}}, {0x01, { 5,15}},

	{0x5a, { 7, 7}}, {0x3f, { 8,19}}, {0x2c, { 9,21}},
	{0x20, {10,22}}, {0x17, {11,23}}, {0x11, {12,25}},
	{0x0c, {13,26}}, {0x09, {14,28}}, {0x07, {15,29}},
	{0x05, {16,31}}, {0x04, {17,32}}, {0x03, {18,34}},
	{0x02, { 5,35}},

	{0x5a, {20,20}}, {0x48, {21,39}}, {0x3a, {22,40}},
	{0x2e, {23,42}}, {0x26, {24,44}}, {0x1f, {25,45}},
	{0x19, {26,46}}, {0x15, {27,25}}, {0x11, {28,26}},
	{0x0e, {29,26}}, {0x0b, {30,27}}, {0x09, {31,28}},
	{0x08, {32,29}}, {0x07, {33,30}}, {0x05, {34,31}},
	{0x04, {35,33}}, {0x04, {36,33}}, {0x03, {37,34}},
	{0x02, {38,35}}, {0x02, { 5,36}},

	{0x58, {40,39}}, {0x4d, {41,47}}, {0x43, {42,48}},
	{0x3b, {43,49}}, {0x34, {44,50}}, {0x2e, {45,51}},
	{0x29, {46,44}}, {0x25, {24,45}},

	{0x56, {48,47}}, {0x4f, {49,47}}, {0x47, {50,48}},
	{0x41, {51,49}}, {0x3c, {52,50}}, {0x37, {43,51}},
};