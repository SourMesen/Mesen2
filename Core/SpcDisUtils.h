#pragma once
#include "stdafx.h"

class DisassemblyInfo;

class SpcDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr);
	static uint8_t GetOpSize(uint8_t opCode);
};
