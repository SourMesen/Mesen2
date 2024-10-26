#pragma once
#include "pch.h"

/*

This file contains a refactored (and somewhat optimized/harder to read) version of the original code by zaydlang
Original is here: https://github.com/zaydlang/multiplication-algorithm/

Copyright (c) 2024 zaydlang

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

	 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
	 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
	 3. This notice may not be removed or altered from any source distribution.

*/

struct MultiplicationOutput
{
	uint64_t Output;
	bool Carry;
	int CycleCount;
};

struct BoothRecodingOutput
{
	uint64_t recoded_output;
	bool carry;
};

struct CSAOutput
{
	uint64_t output;
	uint64_t carry;
};

class GbaCpuMultiply
{
private:
	enum class MultiplicationFlavor
	{
		SHORT,
		LONG_SIGNED,
		LONG_UNSIGNED,
	};

	struct AdderOutput
	{
		uint32_t output;
		bool carry;
	};

	struct u128
	{
		uint64_t lo;
		uint64_t hi;
	};

	__forceinline static uint64_t mask(int lo, int hi)
	{
		uint64_t size = hi - lo;
		return ((1ull << size) - 1) << lo;
	}

	__forceinline static bool bit(uint64_t x, int n)
	{
		return (x >> n) & 1;
	}

	__forceinline static uint64_t sign_extend(uint64_t value, int from_size, int to_size)
	{
		bool negative = bit(value, from_size - 1);
		if(negative) value |= mask(from_size, to_size);
		return value;
	};

	__forceinline static uint64_t asr(uint64_t value, int shift, int size)
	{
		int64_t sign_extended = (int64_t)sign_extend(value, size, 64);
		sign_extended >>= shift;
		return sign_extended & mask(0, size);
	}

	__forceinline static CSAOutput perform_csa_array(uint64_t partial_sum, uint64_t partial_carry, uint64_t multiplicand, uint64_t multiplier, uint64_t& acc_shift_register)
	{
		CSAOutput csa_output = { partial_sum, partial_carry };
		CSAOutput final_csa_output = { 0, 0 };
		CSAOutput result;

		BoothRecodingOutput addends;

		for(int i = 0; i < 4; i++) {
			booth_recode(addends, multiplicand, (multiplier >> (2 * i)) & 0b111);

			csa_output.output &= 0x1FFFFFFFFULL;
			csa_output.carry &= 0x1FFFFFFFFULL;

			uint64_t recodedOutput = addends.recoded_output & 0x1FFFFFFFFULL;
			result.output = csa_output.output ^ recodedOutput ^ csa_output.carry;

			// Inject the carry caused by booth recoding
			result.carry = ((
				(csa_output.output & recodedOutput) | (recodedOutput & csa_output.carry) | (csa_output.carry & csa_output.output)
			) << 1) | (int)addends.carry;

			// Take the bottom two bits and inject them into the final output.
			// The value of the bottom two bits will not be changed by future
			// addends, because those addends must be at least 4 times as big
			// as the current addend. By directly injecting these two bits, the
			// hardware saves some space on the chip.
			final_csa_output.output |= (result.output & 3) << (2 * i);
			final_csa_output.carry |= (result.carry & 3) << (2 * i);

			// The next CSA will only operate on the upper bits - as explained
			// in the previous comment.
			result.output >>= 2;
			result.carry >>= 2;

			// Perform the magic described in the tables for the handling of TransH
			// and High. acc_shift_register contains the upper 31 bits of the acc
			// in its lower bits.
			uint64_t magic = bit(acc_shift_register, 0) + !bit(csa_output.carry, 32) + !bit(addends.recoded_output, 33);
			result.output |= magic << 31;
			result.carry |= (uint64_t)!bit(acc_shift_register, 1) << 32;
			acc_shift_register >>= 2;

			csa_output = result;
		}

		final_csa_output.output |= csa_output.output << 8;
		final_csa_output.carry |= csa_output.carry << 8;

		return final_csa_output;
	}

	__forceinline static void booth_recode(BoothRecodingOutput& output, uint64_t input, uint8_t booth_chunk)
	{
		switch(booth_chunk) {
			case 0: output.recoded_output = 0; output.carry = false; break;
			case 1: output.recoded_output = input & 0x3FFFFFFFFULL; output.carry = 0; break;
			case 2: output.recoded_output = input & 0x3FFFFFFFFULL; output.carry = 0; break;
			case 3: output.recoded_output = (2 * input) & 0x3FFFFFFFFULL; output.carry = 0; break;
			case 4: output.recoded_output = ~(2 * input) & 0x3FFFFFFFFULL; output.carry = 1; break;
			case 5: output.recoded_output = ~input & 0x3FFFFFFFFULL; output.carry = 1; break;
			case 6: output.recoded_output = ~input & 0x3FFFFFFFFULL; output.carry = 1; break;
			case 7: output.recoded_output = 0; output.carry = 0; break;
		}
	}

	static constexpr bool is_long(MultiplicationFlavor flavor)
	{
		return flavor == MultiplicationFlavor::LONG_SIGNED || flavor == MultiplicationFlavor::LONG_UNSIGNED;
	}

	static constexpr bool is_signed(MultiplicationFlavor flavor)
	{
		return flavor == MultiplicationFlavor::LONG_SIGNED || flavor == MultiplicationFlavor::SHORT;
	}

	template<MultiplicationFlavor flavor>
	static bool should_terminate(uint64_t multiplier)
	{
		if(is_signed(flavor)) {
			return multiplier == 0x1FFFFFFFF || multiplier == 0;
		} else {
			return multiplier == 0;
		}
	}

	static AdderOutput adder(uint32_t a, uint32_t b, bool carry)
	{
		uint32_t output = a + b + carry;
		uint64_t real_output = (uint64_t)a + (uint64_t)b + (uint64_t)carry;
		return { output, output != real_output };
	}

	static u128 u128_ror(u128 input, int shift)
	{
		return {
			(input.lo >> shift) | (input.hi << (64 - shift)),
			(input.hi >> shift) | (input.lo << (64 - shift)),
		};
	}

	template<MultiplicationFlavor flavor>
	__forceinline static MultiplicationOutput booths_multiplication(uint64_t multiplicand, uint64_t multiplier, uint64_t accumulator)
	{
		CSAOutput csa_output = { 0, 0 };

		bool alu_carry_in = multiplier & 1;

		if constexpr(is_signed(flavor)) {
			multiplier = sign_extend(multiplier, 32, 34);
		} else {
			multiplier = multiplier & 0x1FFFFFFFFull;
		}

		if constexpr(is_signed(flavor)) {
			multiplicand = sign_extend(multiplicand, 32, 34);
		} else {
			multiplicand = multiplicand & 0x1FFFFFFFFull;
		}

		csa_output.carry = (multiplier & 1) ? ~(multiplicand) : 0;
		csa_output.output = accumulator;
		
		// contains the current high 31 bits of the acc. this is shifted by 2 after each CSA.
		uint64_t acc_shift_register = accumulator >> 34;

		u128 partial_sum = { 0, 0 };
		u128 partial_carry = { 0, 0 };
		partial_sum.lo = csa_output.output & 1;
		partial_carry.lo = csa_output.carry & 1;

		csa_output.output >>= 1;
		csa_output.carry >>= 1;
		partial_sum = u128_ror(partial_sum, 1);
		partial_carry = u128_ror(partial_carry, 1);

		int num_iterations = 0;
		do {
			csa_output = perform_csa_array(csa_output.output, csa_output.carry, multiplicand, multiplier, acc_shift_register);

			partial_sum.lo |= csa_output.output & 0xFF;
			partial_carry.lo |= csa_output.carry & 0xFF;

			csa_output.output >>= 8;
			csa_output.carry >>= 8;

			partial_sum = u128_ror(partial_sum, 8);
			partial_carry = u128_ror(partial_carry, 8);

			multiplier = asr(multiplier, 8, 33);
			num_iterations++;
		} while(!should_terminate<flavor>(multiplier));
		partial_sum.lo |= csa_output.output;
		partial_carry.lo |= csa_output.carry;

		// we have ror'd partial_sum and partial_carry by 8 * num_iterations + 1
		// we now need to ror backwards, i tried my best to mimic the table, but
		// i'm off by one for whatever reason.
		constexpr int rorLut[5] = { 0, 23, 15, 7, 31 };
		int correction_ror = rorLut[num_iterations];

		partial_sum = u128_ror(partial_sum, correction_ror);
		partial_carry = u128_ror(partial_carry, correction_ror);

		if constexpr(is_long(flavor)) {
			if(num_iterations == 4) {
				AdderOutput adder_output_lo = adder(partial_sum.hi, partial_carry.hi, alu_carry_in);
				AdderOutput adder_output_hi = adder(partial_sum.hi >> 32, partial_carry.hi >> 32, adder_output_lo.carry);

				return {
					((uint64_t)adder_output_hi.output << 32) | adder_output_lo.output,
					(bool)((partial_carry.hi >> 63) & 1),
					num_iterations
				};
			} else {
				AdderOutput adder_output_lo = adder(partial_sum.hi >> 32, partial_carry.hi >> 32, alu_carry_in);

				int shift_amount = 1 + 8 * num_iterations;

				// why this is needed is unknown, but the multiplication doesn't work
				// without it
				shift_amount++;

				partial_carry.lo = sign_extend(partial_carry.lo, shift_amount, 64);
				partial_sum.lo |= acc_shift_register << (shift_amount);

				AdderOutput adder_output_hi = adder(partial_sum.lo, partial_carry.lo, adder_output_lo.carry);
				return {
					((uint64_t)adder_output_hi.output << 32) | adder_output_lo.output,
					(bool)((partial_carry.hi >> 63) & 1),
					num_iterations
				};
			}
		} else {
			if(num_iterations == 4) {
				AdderOutput adder_output = adder(partial_sum.hi, partial_carry.hi, alu_carry_in);
				return {
					adder_output.output,
					(bool)((partial_carry.hi >> 31) & 1),
					num_iterations
				};
			} else {
				AdderOutput adder_output = adder(partial_sum.hi >> 32, partial_carry.hi >> 32, alu_carry_in);
				return {
					adder_output.output,
					(bool)((partial_carry.hi >> 63) & 1),
					num_iterations
				};
			}
		}
	}

public:
	static MultiplicationOutput mul(uint32_t rm, uint32_t rs)
	{
		return booths_multiplication<MultiplicationFlavor::SHORT>(rm, rs, 0);
	}

	static MultiplicationOutput mla(uint32_t rm, uint32_t rs, uint32_t rn)
	{
		return booths_multiplication<MultiplicationFlavor::SHORT>(rm, rs, rn);
	}

	static MultiplicationOutput umull(uint32_t rm, uint32_t rs)
	{
		return booths_multiplication<MultiplicationFlavor::LONG_UNSIGNED>(rm, rs, 0);
	}

	static MultiplicationOutput umlal(uint32_t rdlo, uint32_t rdhi, uint32_t rm, uint32_t rs)
	{
		return booths_multiplication<MultiplicationFlavor::LONG_UNSIGNED>(rm, rs, ((uint64_t)rdhi << 32) | (uint64_t)rdlo);
	}

	static MultiplicationOutput smull(uint32_t rm, uint32_t rs)
	{
		return booths_multiplication<MultiplicationFlavor::LONG_SIGNED>(rm, rs, 0);
	}

	static MultiplicationOutput smlal(uint32_t rdlo, uint32_t rdhi, uint32_t rm, uint32_t rs)
	{
		return booths_multiplication<MultiplicationFlavor::LONG_SIGNED>(rm, rs, (uint64_t)rdhi << 32 | (uint64_t)rdlo);
	}
};