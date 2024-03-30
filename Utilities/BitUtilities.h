#pragma once
#include "pch.h"

class BitUtilities
{
public:
	template<uint8_t bitNumber, typename T>
	__forceinline static void SetBits(T& dst, uint8_t src)
	{
		dst = (dst & ~(0xFF << bitNumber)) | (src << bitNumber);
	}

	template<uint8_t bitNumber, typename T>
	__forceinline static uint8_t GetBits(T value)
	{
		return (uint8_t)(value >> bitNumber);
	}
};