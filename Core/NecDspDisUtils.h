#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class LabelManager;

class NecDspDisUtils
{
public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager);
};
